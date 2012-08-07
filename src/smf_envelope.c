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

#include <assert.h>

#include "smf_envelope.h"
#include "smf_email_address.h"
#include "smf_message.h"
#include "smf_list.h"

#define THIS_MODULE "envelope"

void _recipient_list_destroy(void *data) {
    assert(data);
    smf_email_address_free((SMFEmailAddress_T *)data);
}

 /** Creates a new SMFEnvelope_T object */
SMFEnvelope_T *smf_envelope_new(void) {
    SMFEnvelope_T *envelope = NULL;

    envelope = (SMFEnvelope_T *)calloc((size_t)1, sizeof(SMFEnvelope_T));
    
    if (smf_list_new(&envelope->recipients,_recipient_list_destroy)!=0) {
        free(envelope);
        return NULL;
    }

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
    assert(envelope);

    if (envelope->sender != NULL) 
        smf_email_address_free(envelope->sender);
    
    smf_list_free(envelope->recipients);
    
    if (envelope->nexthop != NULL)
        free(envelope->nexthop);

    if (envelope->message != NULL)
        smf_message_free(envelope->message);
    
    if (envelope->message_file != NULL)    
        free(envelope->message_file);    
    
    if (envelope->auth_user != NULL)
        free(envelope->auth_user);    
    
    if (envelope->auth_pass != NULL)
        free(envelope->auth_pass);    
    
    free(envelope);
}

/** Set envelope sender */
void smf_envelope_set_sender(SMFEnvelope_T *envelope, char *sender) {
    assert(envelope);
    assert(sender);
    // free sender, if already set...
    if (envelope->sender != NULL)
        smf_email_address_free(envelope->sender);
    
    envelope->sender = smf_email_address_parse_string(sender);
}

char *smf_envelope_get_sender_string(SMFEnvelope_T *envelope) {
    char *s = NULL;
    assert(envelope);

    if (envelope->sender != NULL)
        s = smf_email_address_to_string(envelope->sender);

    return s;
}

SMFEmailAddress_T *smf_envelope_get_sender(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->sender;
}

/** Add new recipient to envelope */
int smf_envelope_add_rcpt(SMFEnvelope_T *envelope, char *rcpt) {
    SMFEmailAddress_T *ea = NULL;

    assert(envelope);
    assert(rcpt);

    ea = smf_email_address_parse_string(rcpt);
    smf_email_address_set_type(ea, SMF_EMAIL_ADDRESS_TYPE_TO);
    if (envelope->recipients == NULL) {
        if (smf_list_new(&envelope->recipients,_recipient_list_destroy) != 0)
            return -1;
    }

    if (smf_list_append(envelope->recipients,ea) != 0)
        return -1;

    return 0; 
}

void smf_envelope_foreach_rcpt(SMFEnvelope_T *envelope, 
        SMFRcptForeachFunc callback, void  *user_data) {
    SMFListElem_T *elem = NULL;
    

    elem = smf_list_head(envelope->recipients);
    while (elem != NULL) {
        SMFEmailAddress_T *ea = smf_list_data(elem);
        (*callback)(ea,user_data);
    elem = elem->next;
    }

}

/** Set message file path */
void smf_envelope_set_message_file(SMFEnvelope_T *envelope, char *fp) {
    assert(envelope);
    assert(fp);
    if (envelope->message_file != NULL) {
        free(envelope->message_file);
    }
    
    envelope->message_file = strdup(fp);
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
        free(envelope->auth_user);
    }
    
    envelope->auth_user = strdup(auth_user);
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
        free(envelope->auth_pass);
    }
    
    envelope->auth_pass = strdup(auth_pass);
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
        free(envelope->nexthop);
    }
    
    envelope->nexthop = g_strdup(nexthop);
}

/** Get nexthop */
char *smf_envelope_get_nexthop(SMFEnvelope_T *envelope) {
    assert(envelope);
    return envelope->nexthop;
}
