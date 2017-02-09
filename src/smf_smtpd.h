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

#ifndef _SMF_SMTPD_H
#define _SMF_SMTPD_H

#include "smf_settings.h"
#include "smf_session.h"
#include "smf_modules.h"
#include "smf_server.h"

#define CODE_221 "221 Goodbye. Please recommend us to others!\r\n"
#define CODE_250 "250 OK\r\n"
#define CODE_250_ACCEPTED "250 OK message accepted\r\n"
#define CODE_451 "451 Requested action aborted: local error in processing\r\n"
#define CODE_502 "502 Command not implemented\r\n"
#define CODE_552 "552 Requested action aborted: local error in processing\r\n"

/* SMTP States */
#define ST_INIT 0
#define ST_HELO 1
#define ST_XFWD 2
#define ST_MAIL 3
#define ST_RCPT 4
#define ST_DATA 5
#define ST_QUIT 6

static int smf_smtpd_handle_q_error(SMFSettings_T *settings, SMFSession_T *session);
static int smf_smtpd_handle_q_processing_error(SMFSettings_T *settings, SMFSession_T *session, int retval);
static int smf_smtpd_handle_nexthop_error(SMFSettings_T *settings, SMFSession_T *session);
int smf_smtpd_process_modules(SMFSession_T *session, SMFSettings_T *settings, SMFProcessQueue_T *q);
char *smf_smtpd_get_req_value(char *req, int jmp);
void smf_smtpd_stuffing(char chain[]);
int smf_smtpd_append_missing_headers(SMFSession_T *session, 
    char *queue_dir, 
    int mid, 
    int to, 
    int from, 
    int date, 
    int headers, 
    char *nl);
void smf_smtpd_string_reply(SMFServerClient_T *client, const char *format, ...);
void smf_smtpd_code_reply(struct bufferevent *incoming, int code, SMFDict_T *codes);
void smf_smtpd_process_data(SMFSession_T *session, SMFSettings_T *settings,SMFProcessQueue_T *q);
//void smf_smtpd_handle_client(SMFSettings_T *settings, int client,SMFProcessQueue_T *q);
void smf_smtpd_handle_client(struct bufferevent *bev, void *arg);

#endif  /* _SMF_SMTPD_H */

