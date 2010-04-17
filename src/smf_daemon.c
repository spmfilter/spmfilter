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
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <glib.h>

#include "smf_daemon.h"
#include "smf_settings.h"
#include "smf_trace.h"


#define THIS_MODULE "daemon"

volatile sig_atomic_t main_stop = 0;
volatile sig_atomic_t main_restart = 0;
volatile sig_atomic_t main_status = 0;
volatile sig_atomic_t main_sig = 0;

int isChildProcess = 0;

static int smf_daemon_bind(int sock, struct sockaddr *saddr, socklen_t len, int backlog) {
	int err;
	/* bind the address */
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

	TRACE(TRACE_DEBUG, "done");
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

	memset(service, 0, sizeof(char));
	snprintf(service, sizeof(char), "%d", port);

	n = getaddrinfo(ip, service, &hints, &res);
	if (n < 0) {
		TRACE(TRACE_FATAL, "getaddrinfo::error [%s]", gai_strerror(n));
		return -1;
	}

	ressave = res;
	if ((sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0) {
		int serr = errno;
		freeaddrinfo(ressave);
		TRACE(TRACE_FATAL, "%s", strerror(serr));
	}

	TRACE(TRACE_DEBUG, "create socket [%s:%d] backlog [%d]", ip, port, backlog);

	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddress, sizeof(so_reuseaddress));

	smf_daemon_bind(sock, res->ai_addr, res->ai_addrlen, backlog);
	freeaddrinfo(ressave);

	// unblock
	flags = fcntl(sock, F_GETFL);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);

	return sock;
}

int smf_daemon_run(void) {
	int main_stop = 0;
	int main_restart = 0;
	int main_status = 0;
	int main_sig = 0;
	int serrno, status, result = 0;
	pid_t pid = -1;
	SMFSettings_T *settings = smf_settings_get();

	smf_daemon_create_inet_socket(settings->bind_ip,settings->bind_port,BACKLOG);

	switch ((pid = fork())) {
	case -1:
		serrno = errno;
//		smf_daemon_close_sockets();
		TRACE(TRACE_FATAL, "fork failed [%s]", strerror(serrno));
		errno = serrno;
		break;

	case 0:
		/* child process */
		isChildProcess = 1;
		if (drop_privileges(conf->serverUser, conf->serverGroup) < 0) {
			mainStop = 1;
			TRACE(TRACE_ERROR,"unable to drop privileges");
			return 0;
		}

		result = StartServer(conf);
		TRACE(TRACE_INFO, "server done, restart = [%d]",
				result);
		exit(result);
		break;
}

pid_t smf_daemon_daemonize(void) {
	// double-fork
	if (fork()) exit(0);
	setsid();
	if (fork()) exit(0);

	chdir("/");
	umask(0077);

	TRACE(TRACE_DEBUG, "sid: [%d]", getsid(0));

	return getsid(0);
}

int smf_daemon_set_sighandler() {
	struct sigaction act;

	/* init & install signal handlers */
	memset(&act, 0, sizeof(act));

	act.sa_sigaction = smf_daemon_set_sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	sigaction(SIGINT, &act, 0);
	sigaction(SIGQUIT, &act, 0);
	sigaction(SIGTERM, &act, 0);
	sigaction(SIGHUP, &act, 0);
	sigaction(SIGUSR1, &act, 0);

	return 0;
}

void smf_daemon_clear_config(SMFDaemonConfig *config) {
	assert(config);

	g_strfreev(config->iplist);
	g_free(config->listenSockets);

	config->listenSockets = NULL;
	config->iplist = NULL;

	memset(config, 0, sizeof(SMFDaemonConfig_T));
}


char *smf_daemon_get_pidfile(SMFDaemonConfig_T *config) {
	char *res;
	GString *s;
	res = g_build_filename(config->pid_dir, "spmfilter", NULL);
	s = g_string_new("");
	g_string_printf(s, "%s%s", res, DEFAULT_PID_EXT);
	g_free(res);
	res = s->str;
	g_string_free(s,FALSE);
	return res;
}

void smf_daemon_load_config(SMFDaemonConfig_T *config) {
	GError *error = NULL;
	GKeyFile *keyfile;
	int ip;
	SMFSettings_T *settings = smf_settings_get();

	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		exit(-1);
	}

	config->start_children = g_key_file_get_integer(keyfile, "daemon", "start_children",NULL);
	if (!config->start_children) {
		config->start_children = 3;
	}

	TRACE(TRACE_DEBUG,"server will create  [%d] children",config->start_children);

	config->child_max_connect = g_key_file_get_integer(keyfile, "daemon", "max_connetcs",NULL);
	if (!config->child_max_connect) {
		config->child_max_connect = 15;
	}
	TRACE(TRACE_DEBUG, "children will make max. [%d] connections",config->child_max_connect);


	config->timeout = g_key_file_get_integer(keyfile, "daemon", "timeout",NULL);
	if (!config->timeout) {
		config->timeout = 60;
	}
	TRACE(TRACE_DEBUG, ""timeout [%d] seconds",config->timeout);

	config->port = g_key_file_get_integer(keyfile, "daemon", "port", NULL);
	if (!config->port) {
		config->port = 1025;
	}
	TRACE(TRACE_DEBUG, "binding to PORT [%d]",config->port);

	// If there was a SIGHUP, then we're resetting an active config.
	g_strfreev(config->iplist);
	g_free(config->listenSockets);

	config->iplist = g_key_file_get_string_list(keyfile,"daemon","bindip",&config->ipcount,NULL);
	if (config->ipcount < 1) {
		TRACE(TRACE_FATAL, "no value for bindip in config file");
	}

	for (ip = 0; ip < config->ipcount; ip++) {
		// Remove whitespace from each list entry, then log it.
		g_strstrip(config->iplist[ip]);
		TRACE(TRACE_DEBUG, "binding to IP [%s]", config->iplist[ip]);
	}

	config->backlog = g_key_file_get_integer(keyfile, "daemon", "backlog", NULL);
	if (!config->backlog) {
		TRACE(TRACE_DEBUG, "no value for BACKLOG in config file. Using default value [%d]",
			BACKLOG);
		config->backlog = BACKLOG;
	}

	config->effective_user = g_key_file_get_string(keyfile, "daemon", "effective_user", NULL);
	if (strlen(config->effective_user) == 0)
		TRACE(TRACE_FATAL, "no value for EFFECTIVE_USER in config file");

	config->effective_group = g_key_file_get_string(keyfile, "daemon", "effective_group", NULL);
	if (strlen(config->effective_group) == 0)
		TRACE(TRACE_FATAL, "no value for EFFECTIVE_GROUP in config file");

	config->min_spare_children = g_key_file_get_integer(keyfile, "daemon", "min_spare_children", NULL);
	if (!config->min_spare_children) {
		config->min_spare_children = 2;
	}
	TRACE(TRACE_DEBUG, "will maintain minimum of [%d] spare children in reserve",
		config->min_spare_children;
	
	config->max_spare_children = g_key_file_get_integer(keyfile, "daemon", "max_spare_children", NULL);
	if (!config->max_spare_children) {
		config->max_spare_children = 4;
	}
	TRACE(TRACE_DEBUG, "will maintain maximum of [%d] spare children in reserve",
		config->max_spare_children;

	config->max_children = g_key_file_get_integer(keyfile, "daemon", "max_children", NULL);
	if (!config->max_children) {
		config->max_children = 10;
	}
	TRACE(TRACE_DEBUG, "will allow maximum of [%d] children",
		config->max_children;
	
	g_key_file_free(keyfile);
}

int smf_daemon_mainloop(void) {
	SMFDaemonConfig_T config;

	memset(&config, 0, sizeof(SMFDaemonConfig_T));

	smf_daemon_set_sighandler();

	smf_daemon_load_config(&config);
	/* We write the pidFile after daemonize because
	 * we may actually be a child of the original process. */
	if (! config->pid_file)
		config->pid_file = smf_daemon_get_pidfile(config);

	smf_daemon_daemonize();


	smf_daemon_set_sighandler();



	/* This is the actual main loop. */
	while (!main_stop && smf_daemon_run()) {
		sleep(2);
	}

	TRACE(TRACE_INFO, "leaving main loop");
	return 0;
}