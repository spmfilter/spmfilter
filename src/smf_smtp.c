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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <openssl/ssl.h>
#include <auth-client.h>
#include <signal.h>
#include <libesmtp.h>
#include <errno.h>

#include "smf_core.h"
#include "smf_trace.h"
#include "smf_message.h"
#include "smf_envelope.h"
#include "smf_settings.h"
#include "smf_list.h"
#include "smf_smtp.h"

#define THIS_MODULE "smtp"

void smf_smtp_event_cb (smtp_session_t session, int event_no, void *arg, ...);
int smf_smtp_handle_invalid_peer_certificate(long vfy_result);
static int smf_smtp_authinteract (auth_client_request_t request, char **result, int fields, void *arg);
void smf_smtp_print_recipient_status (smtp_recipient_t recipient, const char *mailbox, void *arg);

SMFSmtpStatus_T *smf_smtp_status_new(void) {
    SMFSmtpStatus_T *status = NULL;

    status = (SMFSmtpStatus_T *)calloc((size_t)1, sizeof(SMFSmtpStatus_T));

    return status;
}

void smf_smtp_status_free(SMFSmtpStatus_T *status) {
    assert(status);
    if (status->text != NULL) free(status->text);
    free(status);
}

static int smf_smtp_authinteract (auth_client_request_t request, char **result, int fields, void *arg) {
    int i;

    SMFEnvelope_T *env = (SMFEnvelope_T *)arg;
    for (i = 0; i < fields; i++) {
        if (request[i].flags & AUTH_USER)
            result[i] = env->auth_user;  
        else if (request[i].flags & AUTH_PASS)
            result[i] = env->auth_pass;
        else
            return 0;
    }

  return 1;
} 

/* Callback to prnt the recipient status */
void smf_smtp_print_recipient_status (smtp_recipient_t recipient, const char *mailbox, void *arg) {
    const smtp_status_t *status;
    char *sid = (char *)arg;

    status = smtp_recipient_status(recipient);
    if (sid != NULL)
        STRACE(TRACE_DEBUG,sid,"recipient [%s]: %d %s", mailbox, status->code, status->text);
    else
        TRACE(TRACE_DEBUG,"recipient [%s]: %d %s", mailbox, status->code, status->text);
}

int smf_smtp_handle_invalid_peer_certificate(long vfy_result) {
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
    TRACE(TRACE_DEBUG,"SMTP_EV_INVALID_PEER_CERTIFICATE: %ld: %s", vfy_result, k);
    return 1; /* Accept the problem */
}


void smf_smtp_event_cb (smtp_session_t session, int event_no, void *arg,...) {
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
        case SMTP_EV_EXTNA_STARTTLS:
            TRACE(TRACE_DEBUG, "StartTLS extension not supported by MTA");
            break;
        case SMTP_EV_WEAK_CIPHER: {
            int bits;
            bits = va_arg(alist, long); ok = va_arg(alist, int*);
            TRACE(TRACE_DEBUG,"SMTP_EV_WEAK_CIPHER, bits=%d - accepted.", bits);
            *ok = 1; break;
        }
        case SMTP_EV_STARTTLS_OK:
            TRACE(TRACE_DEBUG,"SMTP_EV_STARTTLS_OK - TLS started here."); break;
        case SMTP_EV_INVALID_PEER_CERTIFICATE: {
            long vfy_result;
            vfy_result = va_arg(alist, long); ok = va_arg(alist, int*);
            TRACE(TRACE_DEBUG, "Invalid peer certificate (error %ld)", vfy_result);
            *ok = smf_smtp_handle_invalid_peer_certificate(vfy_result);
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
            TRACE(TRACE_DEBUG,"Got event: %d - ignored", event_no);
    }
    va_end(alist);
}

SMFSmtpStatus_T *smf_smtp_deliver(SMFEnvelope_T *env, SMFTlsOption_T tls, char *msg_file, char *sid) {
    smtp_session_t session;
    smtp_message_t message;
    smtp_recipient_t recipient;
    auth_context_t authctx = NULL;
    struct sigaction sa;
    const smtp_status_t *retstat;
    SMFListElem_T *elem = NULL;
    char *reverse_path = NULL;
    char *msg_string = NULL;
    FILE *fp = NULL;
    char *s = NULL;
    SMFSmtpStatus_T *status = smf_smtp_status_new();

    assert(env);

    if (sid != NULL)
        STRACE(TRACE_DEBUG,sid,"initializing SMTP session");
    else
        TRACE(TRACE_DEBUG,"initializing SMTP session");

    status->code = -1;

    auth_client_init(); 
    
    session = smtp_create_session();
    message = smtp_add_message(session);

    sa.sa_handler = SIG_IGN;
    sigemptyset (&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction (SIGPIPE, &sa, NULL);

    if (env->nexthop != NULL) {
        if (strstr(env->nexthop,":"))
            smtp_set_server(session, env->nexthop);
        else {
            char *nexthop;
            asprintf(&nexthop,"%s:25",env->nexthop);
            smtp_set_server(session,nexthop);
            free(nexthop);
        }
    } else {
        smtp_destroy_session(session);
        status->code = -1;
        asprintf(&status->text,"invalid smtp host");
        if (sid != NULL)
            STRACE(TRACE_ERR,sid,status->text);
        else
            TRACE(TRACE_ERR,status->text);
        return status;
    }


    smtp_starttls_enable(session,tls);
    smtp_set_eventcb(session, smf_smtp_event_cb, NULL);

    if ((env->auth_user != NULL) && (env->auth_pass != NULL)) {
        authctx = auth_create_context();
        auth_set_mechanism_flags(authctx, AUTH_PLUGIN_PLAIN, 0);
        auth_set_interact_cb(authctx, smf_smtp_authinteract, env);
        smtp_auth_set_context (session, authctx);
    }

    if(env->sender != NULL)  {
        reverse_path = strdup(env->sender);
    } else {
        /* bounce sender */
        asprintf(&reverse_path,"<>");
    }

    if (smtp_set_reverse_path(message,reverse_path) == 0) {
        asprintf(&status->text,"failed to set reverse_path");
        status->code = -1;
        free(reverse_path);
        if (sid != NULL)
            STRACE(TRACE_ERR,sid,status->text);
        else
            TRACE(TRACE_ERR,status->text);
        return status;
    }
    
    free(reverse_path);

    if (msg_file != NULL) {
        if((fp = fopen(msg_file, "r"))==NULL) {
            asprintf(&status->text,"unable to open file: %s (%d)",strerror(errno), errno);
            status->code = -1;
            if (sid != NULL)
                STRACE(TRACE_ERR,sid,status->text);
            else
                TRACE(TRACE_ERR,status->text);
            smtp_destroy_session(session);
            return status;
        }
        smtp_set_message_fp(message, fp);
    } else {
        if (env->message != NULL) {
            msg_string = smf_message_to_string(env->message);
            if (smtp_set_message_str(message,msg_string)==0) {
                asprintf(&status->text,"failed to create message object");
                status->code = -1;
                if (sid != NULL)
                    STRACE(TRACE_ERR,sid,status->text);
                else
                    TRACE(TRACE_ERR,status->text);
                return status;
            }
        } else {
            asprintf(&status->text,"no message content provided");
            status->code = -1;
            if (sid != NULL)
                STRACE(TRACE_ERR,sid,status->text);
            else
                TRACE(TRACE_ERR,status->text);
            smtp_destroy_session(session);
            return status;
        }
    } 
    

    if (env->recipients->size == 0) {
        asprintf(&status->text,"no recipients provided");
        status->code = -1;
        if (sid != NULL)
            STRACE(TRACE_ERR,sid,status->text);
        else
            TRACE(TRACE_ERR,status->text);
        smtp_destroy_session(session);
        if (fp != NULL) fclose(fp);
        return status;
    }

    elem = smf_list_head(env->recipients);
    while(elem != NULL) {
        s = (char *)smf_list_data(elem);
        recipient = smtp_add_recipient(message,s);
        elem = elem->next;
    }

    if (!smtp_start_session(session)) {
        asprintf(&status->text,"failed to initialize smtp session");
        status->code = -1;
        if (sid != NULL)
            STRACE(TRACE_ERR,sid,status->text);
        else
            TRACE(TRACE_ERR,status->text);
        smtp_destroy_session(session);
        if (fp != NULL) fclose(fp);
        return status;
    } else {
        retstat = smtp_message_transfer_status(message);
        smtp_enumerate_recipients(message, smf_smtp_print_recipient_status, sid);
        status->text = (retstat->text != NULL) ? strdup(retstat->text) : NULL;
        status->code = retstat->code;
        
        if (sid != NULL)
            STRACE(TRACE_DEBUG,sid,"smtp client got status '%d - %s'",status->code,status->text);
        else
            TRACE(TRACE_DEBUG,"smtp client got status '%d - %s'",status->code,status->text);
    }

    smtp_destroy_session(session);

    if (fp != NULL) fclose(fp);
    if (msg_string != NULL) free(msg_string);
    if (authctx != NULL) {
        auth_destroy_context(authctx);
        auth_client_exit();
    }

    return status;
}
