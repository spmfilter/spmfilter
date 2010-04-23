/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <glib.h>

#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_daemon.h"
#include "smf_modules.h"

#define THIS_MODULE "daemon"

static int nchildren;

pid_t smf_daemon_daemonize(void) {
	int sid;

	// double-fork
	if (fork()) exit(0);

	sid = setsid();
	chdir("/");
	umask(0);

	if (fork()) exit(0);
	
	TRACE(TRACE_DEBUG, "daemon sid: [%d]", sid);

	return sid;
}

int smf_daemon_load_config(SMFDaemonConfig_T *config) {
	GError *error = NULL;
	GKeyFile *keyfile;
	int ip;
	SMFSettings_T *settings = smf_settings_get();

	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		return -1;
	}

	config->start_children = g_key_file_get_integer(keyfile, "daemon", "start_children",NULL);
	if (!config->start_children) {
		config->start_children = 3;
	}

	TRACE(TRACE_DEBUG,"daemon->start_children: %d",config->start_children);

	config->process_limit = g_key_file_get_integer(keyfile, "daemon", "process_limit",NULL);
	if (!config->process_limit) {
		config->process_limit = 15;
	}
	TRACE(TRACE_DEBUG, "daemon->process_limit",config->process_limit);


	config->timeout = g_key_file_get_integer(keyfile, "daemon", "timeout",NULL);
	if (!config->timeout) {
		config->timeout = 60;
	}
	TRACE(TRACE_DEBUG, "daemon->timeout: %d",config->timeout);

	config->port = g_key_file_get_integer(keyfile, "daemon", "bindport", NULL);
	if (!config->port) {
		config->port = 10025;
	}
	TRACE(TRACE_DEBUG, "daemon->port: %d",config->port);

	// If there was a SIGHUP, then we're resetting an active config.
	g_strfreev(config->iplist);
	g_free(config->listen_sockets);

	config->ipcount = 0;
	config->iplist = g_key_file_get_string_list(keyfile,"daemon","bindip",&config->ipcount,NULL);
	if (config->ipcount < 1) {
		TRACE(TRACE_ERR, "no value for bindip in config file");
	}

	for (ip = 0; ip < config->ipcount; ip++) {
		// Remove whitespace from each list entry, then log it.
		g_strstrip(config->iplist[ip]);
		TRACE(TRACE_DEBUG, "daemon->bindip: %s", config->iplist[ip]);
	}

	config->backlog = g_key_file_get_integer(keyfile, "daemon", "backlog", NULL);
	if (!config->backlog) {
		TRACE(TRACE_DEBUG, "no value for backlog in config file, using default");
		config->backlog = BACKLOG;
	}
	TRACE(TRACE_DEBUG,"daemon->backlog: %d", config->backlog);

	config->effective_user = g_key_file_get_string(keyfile, "daemon", "effective_user", NULL);
	if (config->effective_user == NULL)
		TRACE(TRACE_ERR, "no value for EFFECTIVE_USER in config file");

	TRACE(TRACE_DEBUG,"daemon->effective_user: %s", config->effective_user);

	config->effective_group = g_key_file_get_string(keyfile, "daemon", "effective_group", NULL);
	if (config->effective_group == NULL)
		TRACE(TRACE_ERR, "no value for EFFECTIVE_GROUP in config file");

	TRACE(TRACE_DEBUG,"daemon->effective_group: %s", config->effective_group);

	config->min_spare_children = g_key_file_get_integer(keyfile, "daemon", "min_spare_children", NULL);
	if (!config->min_spare_children) {
		config->min_spare_children = 2;
	}
	TRACE(TRACE_DEBUG, "daemon->min_spare_children: %d", config->min_spare_children);

	config->max_spare_children = g_key_file_get_integer(keyfile, "daemon", "max_spare_children", NULL);
	if (!config->max_spare_children) {
		config->max_spare_children = 4;
	}
	TRACE(TRACE_DEBUG, "daemon->max_spare_children: %d", config->max_spare_children);

	config->max_children = g_key_file_get_integer(keyfile, "daemon", "max_children", NULL);
	if (!config->max_children) {
		config->max_children = 10;
	}
	TRACE(TRACE_DEBUG, "daemon->max_children: %d", config->max_children);

	config->pid_dir = g_key_file_get_string(keyfile, "daemon", "pid_dir", NULL);
	if (config->pid_dir == NULL)
		TRACE(TRACE_ERR, "no value for PID_DIR in config file");
	
	TRACE(TRACE_DEBUG,"daemon->pid_dir: %s", config->pid_dir);

	config->foreground = g_key_file_get_boolean(keyfile, "daemon", "foreground", NULL);
	TRACE(TRACE_DEBUG,"daemon->foreground: %d", config->foreground);

	g_key_file_free(keyfile);

	return 0;
}

ssize_t write_fd(int fd, void *ptr, size_t nbytes, int sendfd) {
	struct msghdr msg;
	struct iovec iov[1];

	union {
		struct cmsghdr	cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *) CMSG_DATA(cmptr)) = sendfd;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return(sendmsg(fd, &msg, 0));
}

ssize_t read_fd(int fd, void *ptr, size_t nbytes, int *recvfd) {
	struct msghdr msg;
	struct iovec iov[1];
	ssize_t n;

	union {
		struct cmsghdr	cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ( (n = recvmsg(fd, &msg, 0)) <= 0)
		return(n);

	if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
		cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
		if (cmptr->cmsg_level != SOL_SOCKET)
			TRACE(TRACE_ERR,"control level != SOL_SOCKET");
		if (cmptr->cmsg_type != SCM_RIGHTS)
			TRACE(TRACE_ERR,"control type != SCM_RIGHTS");
		*recvfd = *((int *) CMSG_DATA(cmptr));
	} else
		*recvfd = -1;		/* descriptor was not passed */

	return(n);
}

int smf_daemon_listen(char *host, int port, socklen_t *addrlenp, int backlog) {
	int listenfd, n;
	const int on = 1;
	struct addrinfo	hints, *res, *ressave;
	char *serv;

	serv = g_strdup_printf("%d",port);

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (n = getaddrinfo(host, serv, &hints, &res)) != 0)
		TRACE(TRACE_ERR,"tcp listen error for %s, %s: %s",
			host, serv, gai_strerror(n));
	ressave = res;

	do {
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (listenfd < 0)
			continue;		/* error, try next one */

		if (setsockopt(listenfd, SOL_SOCKET, SOL_SOCKET, &on, sizeof(on)) < 0)
			TRACE(TRACE_ERR,"setsockopt error");
		if (bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
			break;			/* success */

		close(listenfd);	/* bind error, close and try next one */
	} while ( (res = res->ai_next) != NULL);

	if (res == NULL)	/* errno from final socket() or bind() */
		TRACE(TRACE_ERR, "tcp listen error for %s, %s", host, serv);

	if (listen(listenfd, backlog) < 0)
		TRACE(TRACE_ERR, "listen error");

	if (addrlenp)
		*addrlenp = res->ai_addrlen;	/* return size of protocol address */

	freeaddrinfo(ressave);

	return(listenfd);
}

void smf_daemon_child(int i, int listenfd, int addrlen) {
	char c;
	int connfd;
	ssize_t n;
	SMFSettings_T *settings = smf_settings_get();
	
	TRACE(TRACE_DEBUG, "child %ld starting\n", (long) getpid());
	for (;;) {
		if ((n = read_fd(STDERR_FILENO, &c, 1, &connfd)) == 0)
			TRACE(TRACE_ERR,"read_fd returned 0");
		if (connfd < 0)
			TRACE(TRACE_ERR,"no descriptor from read_fd");

		smf_modules_engine_load(settings, connfd);
		close(connfd);
		write(STDERR_FILENO, "", 1);
	}
}

pid_t smf_daemon_make_child(int i, int listenfd, int addrlen) {
	int	sockfd[2];
	pid_t pid;
	void child_main(int, int, int);

	socketpair(AF_LOCAL, SOCK_STREAM, 0, sockfd);

	if ( (pid = fork()) > 0) {
		close(sockfd[1]);
		cptr[i].child_pid = pid;
		cptr[i].child_pipefd = sockfd[0];
		cptr[i].child_status = 0;
		return(pid); /* parent */
	}

	dup2(sockfd[1], STDERR_FILENO); /* child's stream pipe to parent */
	close(sockfd[0]);
	close(sockfd[1]);
	close(listenfd); /* child does not need this open */
	smf_daemon_child(i, listenfd, addrlen); /* never returns */

	return 0;
}

void sig_int(int signo) {
	int i;

	/* terminate all children */
	for (i = 0; i < nchildren; i++)
		kill(cptr[i].child_pid, SIGTERM);
	while (wait(NULL) > 0)		/* wait for all children */
		;
	if (errno != ECHILD)
		TRACE(TRACE_ERR,"wait error");

	for (i = 0; i < nchildren; i++)
		TRACE(TRACE_DEBUG,"child %d, %ld connections\n", i, cptr[i].child_count);

	exit(0);
}

int smf_daemon_mainloop(SMFSettings_T *settings) {
	SMFDaemonConfig_T config;
	int listenfd, i, navail, nsel, connfd, rc;
	int maxfd = -1;
	void sig_int(int);
	pid_t child_make(int, int, int);
	ssize_t n;
	fd_set rset, masterset;
	socklen_t addrlen, clilen;
	struct sockaddr	*cliaddr;

	memset(&config, 0, sizeof(SMFDaemonConfig_T));
	if (smf_daemon_load_config(&config) != 0) {
		TRACE(TRACE_ERR,"failed to load daemon config");
		return -1;
	}

	if (config.foreground != 1) {
		if (smf_daemon_daemonize() == -1) {
			TRACE(TRACE_DEBUG,"daemonize failed");
			return -1;
		}
	}

	config.listen_sockets = g_new0(int, config.ipcount);
	config.num_sockets = 0;

	FD_ZERO(&masterset);

	listenfd = smf_daemon_listen(SERVER_IP,config.port, &addrlen, config.backlog);
	FD_ZERO(&masterset);
	FD_SET(listenfd, &masterset);
	maxfd = listenfd;

	cliaddr = malloc(addrlen);
	nchildren = config.min_spare_children;

	navail = nchildren;
	cptr = calloc(nchildren, sizeof(SMFDaemonChild_T));

	/* prefork all the children */
	for (i = 0; i < nchildren; i++) {
		smf_daemon_make_child(i, listenfd, addrlen); /* parent returns */
		FD_SET(cptr[i].child_pipefd, &masterset);
		maxfd = max(maxfd, cptr[i].child_pipefd);
	}

	signal(SIGINT, sig_int);

	for (;;) {
		rset = masterset;

		/* turn off if no available children */
		if (navail <= 0) 
				FD_CLR(listenfd, &rset);
		
		nsel = select(maxfd, &rset, NULL, NULL, NULL);

		/* check for new connections */
		if (FD_ISSET(listenfd, &rset)) {
			clilen = addrlen;
			connfd = accept(listenfd, cliaddr, &clilen);

			for (i = 0; i < nchildren; i++)
				if (cptr[i].child_status == 0)
					break;				/* available */

			if (i == nchildren)
				TRACE(TRACE_ERR,"no available children");

			cptr[i].child_status = 1;	/* mark child as busy */
			cptr[i].child_count++;
			navail--;

			n = write_fd(cptr[i].child_pipefd, "", 1, connfd);
			close(connfd);
			if (--nsel == 0)
				continue;	/* all done with select() results */
		}

		/* find any newly-available children */
		for (i = 0; i < nchildren; i++) {
			if (FD_ISSET(cptr[i].child_pipefd, &rset)) {
				if ( (n = read(cptr[i].child_pipefd, &rc, 1)) == 0)
					TRACE(TRACE_ERR,"child %d terminated unexpectedly", i);
				cptr[i].child_status = 0;
				navail++;
				if (--nsel == 0)
					break;	/* all done with select() results */
			}
		}
	}
	
	return 0;
}
