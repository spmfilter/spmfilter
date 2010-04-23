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

#ifndef _SMF_DAEMON_H
#define	_SMF_DAEMON_H

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT	10026
#define BACKLOG 16

//#define	max(a,b) ((a) > (b) ? (a) : (b))

typedef struct {
	char *pid_file;
	char *pid_dir;
	int process_limit;
	int timeout;
	char **iplist;
	unsigned int ipcount;
	int *listen_sockets;
	int num_sockets;
	int port;
	int backlog;
	int socket;
	char *effective_user;
	char *effective_group;
	int foreground;
} SMFDaemonConfig_T;

typedef struct {
	int fd;
	SMFDaemonConfig_T *config;
} SMFDaemonClient_T;

#if 0
typedef struct {
	pid_t child_pid; /* process ID */
	int child_pipefd; /* parent's stream pipe to/from child */
	int child_status; /* 0 = ready */
	long child_count; /* #connections handled */
} SMFDaemonChild_T;

SMFDaemonChild_T *cptr; /* array of Child structures; calloc'ed */
#endif

int smf_daemon_mainloop(SMFSettings_T *settings);

#endif	/* _SMF_DAEMON_H */
