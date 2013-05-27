/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2013 Axel Steiner and SpaceNet AG
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
#include <assert.h>
#include <sys/time.h>

#include "smf_envelope.h"
#include "smf_trace.h"
#include "smf_session.h"
#include "smf_list.h"
#include "smf_internal.h"

#define THIS_MODULE "session"

SMFSession_T *smf_session_new(void) {
    static const char chars[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    SMFSession_T *session;
    int i;
    int pos = 0;
    struct timeval t1;
    
    TRACE(TRACE_DEBUG,"initialize session data");
    session = (SMFSession_T *)calloc((size_t)1, sizeof(SMFSession_T));
    if (smf_list_new(&session->local_users,smf_internal_user_data_list_destroy)!=0) {
        free(session);
        return NULL;
    }

    session->helo = NULL;
    session->xforward_addr = NULL;
    session->message_file = NULL;
    session->message_size = 0;
    session->response_msg = NULL;
    session->envelope = smf_envelope_new();

    /* generate session id */
    gettimeofday(&t1, NULL);

    srandom(t1.tv_usec * t1.tv_sec);
    session->id = (char *)calloc(13,sizeof(char));
    for(i=0; i < 12; i++)
        session->id[pos++] = chars[random() % 36];

    session->id[pos] = '\0';
    TRACE(TRACE_DEBUG,"session->id: [%s]\n",session->id);

    return session;
}

void smf_session_free(SMFSession_T *session) {
    STRACE(TRACE_DEBUG,session->id,"destroy session data");

    if (session->local_users != NULL)
        smf_list_free(session->local_users);

    if (session->helo!=NULL)
        free(session->helo);

    if (session->message_file != NULL)    
        free(session->message_file);  
    
    if (session->xforward_addr!=NULL)
        free(session->xforward_addr);
    
    if (session->response_msg!=NULL)
        free(session->response_msg);
        
    if (session->envelope != NULL)
        smf_envelope_free(session->envelope);
    
    if (session->id!=NULL)
        free(session->id);

    free(session);
}

void smf_session_set_helo(SMFSession_T *session, char *helo) {
    assert(session);
    assert(helo);

    if (session->helo != NULL) {
        free(session->helo);
    }
    
    session->helo = strdup(helo);
}

char *smf_session_get_helo(SMFSession_T *session) {
    assert(session);

    return session->helo;
}

void smf_session_set_message_file(SMFSession_T *session, char *fp) {
    assert(session);
    assert(fp);
    if (session->message_file != NULL) {
        free(session->message_file);
    }
    
    session->message_file = strdup(fp);
}

char *smf_session_get_message_file(SMFSession_T *session) {
    assert(session);
    return session->message_file;
}

void smf_session_set_xforward_addr(SMFSession_T *session, char *xfwd) {
    assert(session);
    assert(xfwd);
    char *s = NULL;

    if (session->xforward_addr != NULL) {
        free(session->xforward_addr);
    }
    
    s = strcasestr(xfwd,"IPv6:");
    if (s) {
        session->xforward_addr = strdup(xfwd + (5 * sizeof(char)));
    } else {
        session->xforward_addr = strdup(xfwd);
    }
}

char *smf_session_get_xforward_addr(SMFSession_T *session) {
    assert(session);

    return session->xforward_addr;
}

void smf_session_set_response_msg(SMFSession_T *session, char *rmsg) {
    assert(session);
    assert(rmsg);

    if (session->response_msg != NULL) {
        free(session->response_msg);
    }
    
    session->response_msg = strdup(rmsg);
}

char *smf_session_get_response_msg(SMFSession_T *session) {
    assert(session);

    return session->response_msg;
}

SMFEnvelope_T *smf_session_get_envelope(SMFSession_T *session) {
    assert(session);

    return session->envelope;
}

char *smf_session_get_id(SMFSession_T *session) {
    assert(session);

    return session->id;
}

int smf_session_is_local(SMFSession_T *session, const char *user) {
    SMFListElem_T *e = NULL;
    SMFUserData_T *d = NULL;
    int retval = 0;

    e = smf_list_head(session->local_users);
    while(e != NULL) {
        d = (SMFUserData_T *)smf_list_data(e);
        if (strcmp(d->email,user) == 0) {
            retval = 1;
            break;
        }

        e = e->next;
    }

    return retval;
}

SMFDict_T *smf_session_get_user_data(SMFSession_T *session, const char *user) {
    SMFListElem_T *e = NULL;
    SMFUserData_T *user_data = NULL;
    
    e = smf_list_head(session->local_users);
    while(e != NULL) {
        user_data = (SMFUserData_T *)smf_list_data(e);
        if (strcmp(user_data->email,user) == 0) {
            return user_data->data;
        }
    }

    return NULL;
}

