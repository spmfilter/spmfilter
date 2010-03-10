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
#include <glib.h>
#include <glib/gstdio.h>

#include <auth-client.h>
#include <signal.h>
#include <libesmtp.h>

#include "smf_core.h"
#include "smf_trace.h"
#include "smf_message.h"

static int authinteract (auth_client_request_t request, char **result, int fields, void *arg);

#define THIS_MODULE "smtp"

/** Deliver message
 *
 * \param msg_data Message_T structure
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_deliver(SMFMessageEnvelope_T *msg_data) {
	smtp_session_t session;
	smtp_message_t message;
	smtp_recipient_t recipient;
	auth_context_t authctx = NULL;
	const smtp_status_t *status;
	char *tmp_file = NULL;
	char *tmp_content = NULL;
	FILE *fp;
	struct sigaction sa;
	char *nexthop = NULL;
	int i;
	
	TRACE(TRACE_DEBUG,"initializing SMTP session");
	
	auth_client_init ();	
	
	session = smtp_create_session();
	message = smtp_add_message(session);
	
	sa.sa_handler = SIG_IGN;
	sigemptyset (&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction (SIGPIPE, &sa, NULL);

	
	if (msg_data->nexthop != NULL) {
		if (g_strrstr(msg_data->nexthop,":"))
			smtp_set_server(session, msg_data->nexthop);
		else {
			nexthop = g_strdup_printf("%s:25", msg_data->nexthop);
			smtp_set_server(session,nexthop);
		}
	} else {
		TRACE(TRACE_ERR,"invalid smtp host");
		smtp_destroy_session(session);
		return -1;
	}
	
	if ((msg_data->auth_user != NULL) && (msg_data->auth_pass != NULL)) {
		authctx = auth_create_context();
		auth_set_mechanism_flags(authctx, AUTH_PLUGIN_PLAIN, 0);
		auth_set_interact_cb(authctx, authinteract, msg_data);
		smtp_auth_set_context (session, authctx);
	}
	
	if(msg_data->from != NULL) {
		smtp_set_reverse_path(message,msg_data->from);
	} else {
		/* bounce sender */
		smtp_set_reverse_path(message, "<>");
	}
	
	if ((msg_data->message_file == NULL) && (msg_data->message != NULL)) {
		smf_core_gen_queue_file(&tmp_file);
		tmp_content = smf_message_to_string(msg_data->message);
		fp = fopen(tmp_file,"w+b");
		fwrite(tmp_content,strlen(tmp_content),1,fp);
		rewind(fp);
		free(tmp_content);
	} else {
		fp = fopen(msg_data->message_file, "r");
	}
	smtp_set_message_fp(message, fp);
	
	if (msg_data->rcpts != NULL) {
		for (i = 0; i < msg_data->num_rcpts; i++) {
			recipient = smtp_add_recipient(message,msg_data->rcpts[i]);
		}
	} else {
		TRACE(TRACE_ERR,"no recipients provided");
		smtp_destroy_session(session);
		fclose(fp);
		return -1;
	}

	if (!smtp_start_session(session)) {
		TRACE(TRACE_ERR,"failed to initialize smtp session");
		smtp_destroy_session(session);
		fclose(fp);
		return -1;
	} else {
		status = smtp_message_transfer_status(message);
		TRACE(TRACE_DEBUG,"smtp client got status '%d - %s'",status->code,status->text);
	}
	
	g_free(nexthop);
	smtp_destroy_session(session);
	fclose (fp);
	
	if (authctx != NULL) {
		auth_destroy_context(authctx);
		auth_client_exit();
	}

	if (tmp_file != NULL) 
		g_remove(tmp_file);

	return 0;
}


static int authinteract (auth_client_request_t request, char **result, int fields, void *arg) {
	int i;
	
	SMFMessageEnvelope_T *msg_data = (SMFMessageEnvelope_T *)arg;
	for (i = 0; i < fields; i++) {
		if (request[i].flags & AUTH_USER)
			result[i] = g_strdup(msg_data->auth_user);	
		else if (request[i].flags & AUTH_PASS)
			result[i] = g_strdup(msg_data->auth_pass);
		else
			return 0;
	}

  return 1;
} 


