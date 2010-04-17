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

#define BACKLOG 16
#define HARD_MAX_CHILDREN 300
#define SOCKADDR_LEN NI_MAXSERV+NI_MAXHOST

typedef struct {
	pid_t pid;
	time_t ctime;
	unsigned char status;
	unsigned long count;
	char client[128];
} SMFChildState_T;

typedef struct {
	char *pid_file;
	char *state_file;
	int start_children;
	int min_spare_children;
	int max_spare_children;
	int max_children;
	int child_max_connect;
	int timeout;
	char **iplist;
	int ipcount;
	int *listen_sockets;
	int port;
	int backlog;
	int socket;
	char *pid_dir;
	char *state_dir;
	char *effective_user;
	char *effective_group;
} SMFDaemonConfig_T;

typedef struct {
	unsigned int lock;
	SMFDaemonConfig_T *config;
	SMFChildState_T[HARD_MAX_CHILDREN];
} SMFScoreBoard_T;

int smf_daemon_mainloop(void);

#endif	/* _SMF_DAEMON_H */
