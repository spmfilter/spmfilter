/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
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

#ifndef _SMF_SERVER_H
#define _SMF_SERVER_H

#include "smf_settings.h"
#include "smf_modules.h"

typedef void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q);

void smf_server_sig_init(void);
void smf_server_sig_handler(int sig);
void smf_server_init(SMFSettings_T *settings, int sd);
int smf_server_listen(SMFSettings_T *settings);
void smf_server_fork(SMFSettings_T *settings,int sd,SMFProcessQueue_T *q,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));
void smf_server_loop(SMFSettings_T *settings,int sd,SMFProcessQueue_T *q,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));
void smf_server_accept_handler(
    SMFSettings_T *settings, 
    int sd, 
    SMFProcessQueue_T *q,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));

#endif  /* _SMF_SERVER_H */

