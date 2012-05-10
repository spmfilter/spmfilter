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

#include "smf_session.h"
#include "smf_session_private.h"

#define THIS_MODULE "session"

SMFSession_T *smf_session_new(void) {
    SMFSession_T *session;
    TRACE(TRACE_DEBUG,"initialize session data");
    session = g_slice_new(SMFSession_T);
    session->helo = NULL;
    session->xforward_addr = NULL;
    session->msgbodysize = 0;
    session->response_msg = NULL;
    session->envelope = smf_envelope_new();
    return session;
}

/** Free SMFSession_T structure */
void smf_session_free(SMFSession_T *session) {
    TRACE(TRACE_DEBUG,"destroy session data");
    g_free(session->helo);
    g_free(session->xforward_addr);
    g_free(session->response_msg);
    smf_envelope_free(session->envelope);
    
    g_slice_free(SMFSession_T,session);
    session = NULL;
}

/** Set helo */
void smf_session_set_helo(SMFSession_T *session, char *helo) {
    assert(session);
    assert(helo);

    if (session->helo != NULL) {
        g_free(session->helo);
    }
    
    session->helo = g_strdup(helo);
}

char *smf_session_get_helo(SMFSession_T *session) {
    assert(session);

    return session->helo;
}


/** Set xforward address */
void smf_session_set_xforward_addr(SMFSession_T *session, char *xfwd) {
    assert(session);
    assert(xfwd);

    if (session->xforward_addr != NULL) {
        g_free(session->xforward_addr);
    }
    
    session->xforward_addr = g_strdup(xfwd);
}

char *smf_session_get_xforward_addr(SMFSession_T *session) {
    assert(session);

    return session->xforward_addr;
}

/** Set response message */
void smf_session_set_response_msg(SMFSession_T *session, char *rmsg) {
    assert(session);
    assert(rmsg);

    if (session->response_msg != NULL) {
        g_free(session->response_msg);
    }
    
    session->response_msg = g_strdup(rmsg);
}

char *smf_session_get_response_msg(SMFSession_T *session) {
    assert(session);

    return session->response_msg;
}

SMFEnvelope_T *smf_session_get_envelope(SMFSession_T *session) {
    assert(session);

    return session->envelope;
}
