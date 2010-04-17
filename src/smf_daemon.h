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

#define SOCKADDR_LEN NI_MAXSERV+NI_MAXHOST

typedef struct {
    FILE *tx, *rx;
    struct sockaddr_storage caddr;
    socklen_t caddr_len;
    char src_ip[NI_MAXHOST];  /* client IP-number */
    char src_port[NI_MAXSERV];/* client IP-port */
    struct sockaddr_storage saddr;
    socklen_t saddr_len;
    char dst_ip[NI_MAXHOST]; /* server IP-number */
    char dst_port[NI_MAXSERV];/* server IP-port */
    char *clientname;	  /* resolved client ip */
    int timeout;		  /* server timeout (seconds) */
} SMFClientInfo_T;

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
    int (*ClientHandler) (SMFClientInfo_T *);
} SMFDaemonConfig_T;

int smf_daemon_mainloop(void);

#endif	/* _SMF_DAEMON_H */

