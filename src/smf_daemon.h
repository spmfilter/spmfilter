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

typedef struct {
	char *pid_file;
	char *pid_dir;
	int max_threads;
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

int smf_daemon_mainloop(SMFSettings_T *settings);

#endif	/* _SMF_DAEMON_H */
