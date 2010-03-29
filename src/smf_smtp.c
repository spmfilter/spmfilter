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
#include <gmime/gmime.h>

#include <openssl/ssl.h>
#include <auth-client.h>
#include <signal.h>
#include <libesmtp.h>

#include "smf_core.h"
#include "smf_trace.h"
#include "smf_message.h"
#include "smf_settings.h"

static int authinteract (auth_client_request_t request, char **result, int fields, void *arg);
static int tlsinteract (char *buf, int buflen, int rwflag, void *arg);
void event_cb (smtp_session_t session, int event_no, void *arg, ...);
int handle_invalid_peer_certificate(long vfy_result);
void print_recipient_status (smtp_recipient_t recipient, const char *mailbox, void *arg);

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
	int i,ret;
	GMimeStream *stream, *stream_filter;
	GMimeFilter *crlf;
	SMFSettings_T *settings = smf_settings_get();

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
	
	smtp_starttls_enable(session,settings->tls);
	smtp_starttls_set_password_cb(tlsinteract, NULL);
	smtp_set_eventcb(session, event_cb, NULL);

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
	if (msg_data->message != NULL) {
		smf_core_gen_queue_file(&tmp_file);
		tmp_content = smf_message_to_string(msg_data->message);
		fp = fopen(tmp_file,"w+");
	
		stream = g_mime_stream_file_new(fp);
#ifdef HAVE_GMIME24
		stream_filter = g_mime_stream_filter_new(stream);
#else
		stream_filter = g_mime_stream_filter_new_with_stream(stream);
#endif
		crlf = g_mime_filter_crlf_new(TRUE,FALSE);
		g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter), crlf);

		g_mime_stream_write_string(stream_filter,tmp_content);
		g_mime_stream_flush(stream);
		g_object_unref(crlf);
		g_object_unref(stream_filter);
		rewind(fp);
		g_free(tmp_content);
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
		smtp_enumerate_recipients(message, print_recipient_status, NULL);
		TRACE(TRACE_DEBUG,"smtp client got status '%d - %s'",status->code,status->text);
		if (status->code != 250)
			ret = -1;
		else
			ret = 0;
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

	return ret;
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

static int tlsinteract(char *buf, int buflen, int rwflag, void *arg) {
	char *pw;
	int len;
	SMFSettings_T *settings = smf_settings_get();

	if (settings->tls_pass) {
		pw = settings->tls_pass;
		len = strlen (pw);
		if (len + 1 > buflen)
			return 0;
		strcpy (buf, pw);
		return len;
	} else
		return 0;
}

void event_cb (smtp_session_t session, int event_no, void *arg,...) {
	va_list alist;
	int *ok;

	va_start(alist, arg);
	switch(event_no) {
		case SMTP_EV_CONNECT:
		case SMTP_EV_MAILSTATUS:
		case SMTP_EV_RCPTSTATUS:
		case SMTP_EV_MESSAGEDATA:
		case SMTP_EV_MESSAGESENT:
		case SMTP_EV_DISCONNECT: break;
		case SMTP_EV_WEAK_CIPHER: {
			int bits;
			bits = va_arg(alist, long); ok = va_arg(alist, int*);
			TRACE(TRACE_DEBUG,"SMTP_EV_WEAK_CIPHER, bits=%d - accepted.\n", bits);
			*ok = 1; break;
		}
		case SMTP_EV_STARTTLS_OK:
			TRACE(TRACE_DEBUG,"SMTP_EV_STARTTLS_OK - TLS started here."); break;
		case SMTP_EV_INVALID_PEER_CERTIFICATE: {
			long vfy_result;
			vfy_result = va_arg(alist, long); ok = va_arg(alist, int*);
			*ok = handle_invalid_peer_certificate(vfy_result);
			break;
		}
		case SMTP_EV_NO_PEER_CERTIFICATE: {
			ok = va_arg(alist, int*);
			TRACE(TRACE_DEBUG,"SMTP_EV_NO_PEER_CERTIFICATE - accepted.");
			*ok = 1; break;
		}
		case SMTP_EV_WRONG_PEER_CERTIFICATE: {
			ok = va_arg(alist, int*);
			TRACE(TRACE_DEBUG,"SMTP_EV_WRONG_PEER_CERTIFICATE - accepted.");
			*ok = 1; break;
		}
		case SMTP_EV_NO_CLIENT_CERTIFICATE: {
			ok = va_arg(alist, int*);
			TRACE(TRACE_DEBUG,"SMTP_EV_NO_CLIENT_CERTIFICATE - accepted.");
			*ok = 1; break;
		}
		default:
			TRACE(TRACE_DEBUG,"Got event: %d - ignored.\n", event_no);
	}
	va_end(alist);
}

int handle_invalid_peer_certificate(long vfy_result) {
	const char *k ="rare error";
	switch(vfy_result) {
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
			k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT"; break;
		case X509_V_ERR_UNABLE_TO_GET_CRL:
			k="X509_V_ERR_UNABLE_TO_GET_CRL"; break;
		case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
			k="X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE"; break;
		case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
			k="X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE"; break;
		case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
			k="X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY"; break;
		case X509_V_ERR_CERT_SIGNATURE_FAILURE:
			k="X509_V_ERR_CERT_SIGNATURE_FAILURE"; break;
		case X509_V_ERR_CRL_SIGNATURE_FAILURE:
			k="X509_V_ERR_CRL_SIGNATURE_FAILURE"; break;
		case X509_V_ERR_CERT_NOT_YET_VALID:
			k="X509_V_ERR_CERT_NOT_YET_VALID"; break;
		case X509_V_ERR_CERT_HAS_EXPIRED:
			k="X509_V_ERR_CERT_HAS_EXPIRED"; break;
		case X509_V_ERR_CRL_NOT_YET_VALID:
			k="X509_V_ERR_CRL_NOT_YET_VALID"; break;
		case X509_V_ERR_CRL_HAS_EXPIRED:
			k="X509_V_ERR_CRL_HAS_EXPIRED"; break;
		case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
			k="X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD"; break;
		case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
			k="X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD"; break;
		case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
			k="X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD"; break;
		case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
			k="X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD"; break;
		case X509_V_ERR_OUT_OF_MEM:
			k="X509_V_ERR_OUT_OF_MEM"; break;
		case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
			k="X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT"; break;
		case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
			k="X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN"; break;
		case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
			k="X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY"; break;
		case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
			k="X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE"; break;
		case X509_V_ERR_CERT_CHAIN_TOO_LONG:
			k="X509_V_ERR_CERT_CHAIN_TOO_LONG"; break;
		case X509_V_ERR_CERT_REVOKED:
			k="X509_V_ERR_CERT_REVOKED"; break;
		case X509_V_ERR_INVALID_CA:
			k="X509_V_ERR_INVALID_CA"; break;
		case X509_V_ERR_PATH_LENGTH_EXCEEDED:
			k="X509_V_ERR_PATH_LENGTH_EXCEEDED"; break;
		case X509_V_ERR_INVALID_PURPOSE:
			k="X509_V_ERR_INVALID_PURPOSE"; break;
		case X509_V_ERR_CERT_UNTRUSTED:
			k="X509_V_ERR_CERT_UNTRUSTED"; break;
		case X509_V_ERR_CERT_REJECTED:
			k="X509_V_ERR_CERT_REJECTED"; break;
	}
	TRACE(TRACE_DEBUG,"SMTP_EV_INVALID_PEER_CERTIFICATE: %ld: %s\n", vfy_result, k);
	return 1; /* Accept the problem */
}

/* Callback to prnt the recipient status */
void print_recipient_status (smtp_recipient_t recipient, const char *mailbox, void *arg) {
	const smtp_status_t *status;

	status = smtp_recipient_status (recipient);
	TRACE(TRACE_DEBUG,"%s: %d %s", mailbox, status->code, status->text);
}
