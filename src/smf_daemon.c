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

volatile sig_atomic_t GeneralStopRequested = 0;

static int smf_daemon_bind(int sock, struct sockaddr *saddr, socklen_t len, int backlog) {
	int err;
	if ((bind(sock, saddr, len)) == -1) {
		err = errno;
		TRACE(TRACE_DEBUG, "failed");
		return err;
	}

	if ((listen(sock, backlog)) == -1) {
		err = errno;
		TRACE(TRACE_DEBUG, "failed");
		return err;
	}

	return 0;
}

static int smf_daemon_create_inet_socket(const char * const ip, int port, int backlog) {
	struct addrinfo hints, *res, *ressave;
	int sock, n, flags;
	int so_reuseaddress = 1;
	char *service;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	service = g_strdup_printf("%d",port);

	n = getaddrinfo(ip, service, &hints, &res);
	if (n < 0) {
		TRACE(TRACE_ERR, "getaddrinfo::error [%s]", gai_strerror(n));
		return -1;
	}

	ressave = res;
	if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
		int serr = errno;
		freeaddrinfo(ressave);
		TRACE(TRACE_ERR, "%s", strerror(serr));
	}

	TRACE(TRACE_DEBUG, "create socket [%s:%d] backlog [%d]", ip, port, backlog);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddress, sizeof(so_reuseaddress));

	smf_daemon_bind(sock, res->ai_addr, res->ai_addrlen, backlog);
	freeaddrinfo(ressave);

	flags = fcntl(sock, F_GETFL);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	return sock;
}

pid_t smf_daemon_daemonize(void) {
	int sid;

	// double-fork
	if (fork()) exit(0);
	sid = setsid();
	if (fork()) exit(0);

	chdir("/");
	umask(0077);

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

	config->max_connects = g_key_file_get_integer(keyfile, "daemon", "max_connetcs",NULL);
	if (!config->max_connects) {
		config->max_connects = 15;
	}
	TRACE(TRACE_DEBUG, "daemon->max_connects",config->max_connects);


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

int smf_daemon_start(SMFDaemonConfig_T *config) {
	if (!config)
		TRACE(TRACE_ERR, "NULL configuration");

	TRACE(TRACE_DEBUG, "starting main service loop");
 	while (!GeneralStopRequested) {
		
	}

	return 0;
}

int smf_daemon_run(SMFDaemonConfig_T *config) {
	int i;
	pid_t pid = -1;
	int serrno, status, result = 0;
	
	config->listen_sockets = g_new0(int, config->ipcount);

	for (i = 0; i < config->ipcount; i++) {
		config->listen_sockets[i] = smf_daemon_create_inet_socket(
			config->iplist[i], config->port, config->backlog);

	}

	switch ((pid = fork())) {
		case -1:
			serrno = errno;
			TRACE(TRACE_ERR, "fork failed [%s]", strerror(serrno));
			errno = serrno;
			break;
		case 0:
			/* child process */
			result = smf_daemon_start(config);
			TRACE(TRACE_INFO, "server done, restart = [%d]", result);
			exit(result);
			break;
		default:
			/* parent process, wait for child to exit */
			while (waitpid(pid, &status, WNOHANG | WUNTRACED) == 0) {
				sleep(2);
			}
			break;		
	}

	return result;
}

int smf_daemon_mainloop(SMFSettings_T *settings) {
	SMFDaemonConfig_T config;
	int i;
	fd_set rfds;
	int maxfd = -1;

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
/*
	while (smf_daemon_run(&config)) {
		sleep(2);
	}
*/

	config.listen_sockets = g_new0(int, config.ipcount);
	config.num_sockets = 0;
	for (i = 0; i < config.ipcount; i++) {
		config.listen_sockets[i] = smf_daemon_create_inet_socket(
			config.iplist[i], config.port, config.backlog);
		if (config.listen_sockets[i] > maxfd)
			maxfd = config.listen_sockets[i];
		config.num_sockets++;
	}


	/*  Loop infinitely to accept and service connections  */
	while ( 1 ) {
		FD_ZERO(&rfds);
		for (i = 0; i < config.num_sockets; i++) 
			FD_SET(config.listen_sockets[i], &rfds);
		

		int returned = select(maxfd + 1, &rfds, NULL, NULL, NULL);
		if (returned) {
			for (i = 0; i < config.num_sockets; i++) {
				if (FD_ISSET(config.listen_sockets[i], &rfds)) {
					int conn;
					pid_t pid;
					/*  Wait for connection  */
					int listener = config.listen_sockets[i];
					if ( (conn = accept(listener, NULL, NULL)) < 0 ) {
						TRACE(TRACE_ERR,"Error calling accept()");
						return -1;
					}

					/*  Fork child process to service connection  */
					if ( (pid = fork()) == 0 ) {
						/* This is now the forked child process, so
						 * close listening socket and service request   */
						if ( close(listener)  < 0 ) {
							TRACE(TRACE_ERR,"Error closing listening socket in child.");
							return -1;
						}

						smf_modules_engine_load(settings, conn);

						/*  Close connected socket and exit  */
						if ( close(conn) < 0 ) {
							TRACE(TRACE_ERR,"Error closing connection socket.");
							return -1;
						}
						exit(EXIT_SUCCESS);
					}

					/* If we get here, we are still in the parent process,
					 * so close the connected socket, clean up child processes,
					 * and go back to accept a new connection.                   */

					if ( close(conn) < 0 ) {
						TRACE(TRACE_ERR,"Error closing connection socket in parent.");
						return -1;
					}
					waitpid(-1, NULL, WNOHANG);
		
				}
			}
		}
	}


	return 0;
}
