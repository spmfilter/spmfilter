#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <glib.h>

#include <auth-client.h>
#include <signal.h>
#include <libesmtp.h>

#include "spmfilter.h"

static int authinteract (auth_client_request_t request, char **result, int fields, void *arg);

int smtp_delivery(SETTINGS *settings, MESSAGE *msg_data) {
	smtp_session_t session;
	smtp_message_t message;
	smtp_recipient_t recipient;
	auth_context_t authctx = NULL;
	const smtp_status_t *status;
	FILE *fp;
	struct sigaction sa;
	char *nexthop = NULL;
	GSList *rcpt;
	
	if (settings->debug)
		syslog(LOG_DEBUG,"initializing SMTP session");
	
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
			asprintf(&nexthop,"%s:25",msg_data->nexthop);
			smtp_set_server(session,nexthop);
		}
	} else {
		smtp_destroy_session(session);
		return -1;
	}
	
	if (msg_data->auth_user != NULL && msg_data->auth_pass != NULL) {
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
	
	
	fp = fopen(msg_data->message_file, "r");
	smtp_set_message_fp(message, fp);
	
	if (msg_data->rcpt != NULL) {
		for (rcpt = msg_data->rcpt; rcpt; rcpt = g_slist_next(rcpt)) {
			recipient = smtp_add_recipient(message,rcpt->data);
		}
	} else {
		smtp_destroy_session(session);
		fclose(fp);
		return -1;
	}

	if (!smtp_start_session(session)) {
		smtp_destroy_session(session);
		fclose(fp);
		return -1;
	} else {
		status = smtp_message_transfer_status(message);
		if (settings->debug) {
			syslog(LOG_DEBUG,"smtp client got status '%d - %s'",status->code,status->text);
		}
	}
	
	g_free(nexthop);
	smtp_destroy_session (session);
	fclose (fp);
	
	if (authctx != NULL) {
		auth_destroy_context(authctx);
		auth_client_exit();
	}
	
	return 0;
}


static int authinteract (auth_client_request_t request, char **result, int fields, void *arg) {
	int i;
	MESSAGE *msg_data = arg;
	
	for (i = 0; i < fields; i++) {
		if (request[i].flags & AUTH_USER)
			result[i] = msg_data->auth_user;	
		else if (request[i].flags & AUTH_PASS)
			result[i] = msg_data->auth_pass;
	}

  return 0;
} 
