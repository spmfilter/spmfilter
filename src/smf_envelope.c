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

#include "smf_envelope.h"
#include "smf_email_address.h"

#define THIS_MODULE "envelope"

 /** Creates a new SMFEnvelope_T object */
SMFEnvelope_T *smf_envelope_new(void) {
    SMFEnvelope_T *envelope = NULL;

    envelope = g_slice_new(SMFEnvelope_T);
    envelope->num_rcpts = 0;
    envelope->rcpt = NULL;
    envelope->sender = NULL;
    envelope->message_file = NULL;
    envelope->message = NULL;
    envelope->auth_pass = NULL;
    envelope->auth_user = NULL;
    envelope->nexthop = NULL;

    return envelope;
}

/** Free SMFEnvelope_T object */
void smf_envelope_free(SMFEnvelope_T *envelope) {
    int i;

    if (envelope->sender != NULL) {
        smf_email_address_free(envelope->sender);
    }

    if (envelope->rcpt != NULL) {
        for (i = 0; i < envelope->num_rcpts; i++) {
            if (envelope->rcpt[i] != NULL) {
                smf_email_address_free(envelope->rcpt[i]);
            }
        }
        g_free(envelope->rcpt);
    }

    g_free(envelope->nexthop);

    //if (envelope->message != NULL)
        // TODO: free envelope->message
    //    smf_message_unref(envelope->message);
        
    g_free(envelope->message_file);    
    g_free(envelope->auth_user);    
    g_free(envelope->auth_pass);    
    g_slice_free(SMFEnvelope_T,envelope);
}

/** Add new recipient to envelope */
void smf_envelope_add_rcpt(SMFEnvelope_T *envelope, char *rcpt) {
    assert(envelope);
    assert(rcpt);
    envelope->rcpt = g_realloc(
        envelope->rcpt,
        sizeof(SMFEmailAddress_T) * (envelope->num_rcpts + 1)
    );

    envelope->rcpt[envelope->num_rcpts] = smf_email_address_new();
//    envelope->rcpt[envelope->num_rcpts]->addr = g_strdup(rcpt);
    envelope->num_rcpts++;
}

void smf_envelope_foreach_rcpt(SMFEnvelope_T *envelope, 
        SMFRcptForeachFunc callback, void  *user_data) {
    int i;
    
    if (envelope->rcpt != NULL) {
        for (i = 0; i < envelope->num_rcpts; i++) {
            (*callback)(envelope->rcpt[i],user_data);
        }
    }       
}

/** Set envelope sender */
void smf_envelope_set_sender(SMFEnvelope_T *envelope, char *sender) {
    assert(envelope);
    assert(sender);
    // free sender, if already set...
    if (envelope->sender != NULL)
        smf_email_address_free(envelope->sender);
    
    envelope->sender = smf_email_address_new();
//    envelope->sender->addr = g_strdup(sender);
}

/** Get envelope sender */
SMFEmailAddress_T *smf_envelope_get_sender(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->sender;
}

/** Set message file path */
void smf_envelope_set_message_file(SMFEnvelope_T *envelope, char *fp) {
    assert(envelope);
    assert(fp);
    if (envelope->message_file != NULL) {
        g_free(envelope->message_file);
    }
    
    envelope->message_file = g_strdup(fp);
}

char *smf_envelope_get_message_file(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->message_file;
}

/** Set auth user */
void smf_envelope_set_auth_user(SMFEnvelope_T *envelope, char *auth_user) {
    assert(envelope);
    assert(auth_user);
    if (envelope->auth_user != NULL) {
        g_free(envelope->auth_user);
    }
    
    envelope->auth_user = g_strdup(auth_user);
}

/** Get auth user */
char *smf_envelope_get_auth_user(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->auth_user;
}

/** Set auth password */
void smf_envelope_set_auth_pass(SMFEnvelope_T *envelope, char *auth_pass) {
    assert(envelope);
    assert(auth_pass);
    if (envelope->auth_pass != NULL) {
        g_free(envelope->auth_pass);
    }
    
    envelope->auth_pass = g_strdup(auth_pass);
}

/** Get auth pass */
char *smf_envelope_get_auth_pass(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->auth_pass;
}

/** Set nexthop */
void smf_envelope_set_nexthop(SMFEnvelope_T *envelope, char *nexthop) {
    assert(envelope);
    assert(nexthop);
    if (envelope->nexthop != NULL) {
        g_free(envelope->nexthop);
    }
    
    envelope->nexthop = g_strdup(nexthop);
}

/** Get nexthop */
char *smf_envelope_get_nexthop(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->nexthop;
}