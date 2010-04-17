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
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <glib.h>

#include "smf_settings.h"
#include "smf_trace.h"

#define BACKLOG 16
#define UNBLOCK(a) \
	{ \
		int flags; \
		if ( (flags = fcntl(a, F_GETFL, 0)) < 0) \
			perror("F_GETFL"); \
		flags |= O_NONBLOCK; \
		if (fcntl(a, F_SETFL, flags) < 0) \
			perror("F_SETFL"); \
	}


#define THIS_MODULE "daemon"

static int smf_daemon_bind_and_listen(int sock, struct sockaddr *saddr, socklen_t len) {
	int err;
	/* bind the address */
	if ((bind(sock, saddr, len)) == -1) {
		err = errno;
		TRACE(TRACE_EMERG, "%s", strerror(err));
	}

	if ((listen(sock, BACKLOG)) == -1) {
		err = errno;
		TRACE(TRACE_EMERG, "%s", strerror(err));
	}

	TRACE(TRACE_DEBUG, "done");
	return 0;
}

int smf_daemon_run(void) {
	int sock, err;
	struct sockaddr_in sa_server;
	int so_reuseaddress = 1;
	SMFSettings_T *settings = smf_settings_get();

	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
		err = errno;
		TRACE(TRACE_EMERG, "%s", strerror(err));
	}


	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddress, sizeof(so_reuseaddress));

	/* setup sockaddr_in */
	memset(&sa_server, 0, sizeof(sa_server));
	sa_server.sin_family	= AF_INET;
	sa_server.sin_port	= htons(settings->bind_port);

	TRACE(TRACE_DEBUG, "create socket [%s:%d] backlog [%d]", settings->bind_ip,
			settings->bind_port, BACKLOG);

	if (settings->bind_ip[0] == '*') {
		sa_server.sin_addr.s_addr = htonl(INADDR_ANY);
	} else if (! (inet_aton(settings->bind_ip, &sa_server.sin_addr))) {
		if (sock > 0) close(sock);
		TRACE(TRACE_EMERG, "IP invalid [%s]", settings->bind_ip);
	}

	// any error in dm_bind_and_listen is fatal
	smf_daemon_bind_and_listen(sock, (struct sockaddr *)&sa_server, sizeof(sa_server));

	UNBLOCK(sock);

	TRACE(TRACE_NOTICE, "starting main service loop for spmfilter");

	srand((int) ((int) time(NULL) + (int) getpid()));

	TRACE(TRACE_DEBUG,"setup event loop");

//	smf_daemon_pidfile();

	return 0;
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

int smf_daemon_mainloop(void) {

	smf_daemon_daemonize();

	/* This is the actual main loop. */
	while (smf_daemon_run()) {
		sleep(2);
	}

	TRACE(TRACE_INFO, "leaving main loop");
	return 0;
}