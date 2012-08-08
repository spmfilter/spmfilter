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

#include <assert.h>
#include <glib.h>
#include <cmime.h>

#include "smf_list.h"
#include "smf_header.h"
#include "smf_message.h"
#include "smf_part.h"


#define THIS_MODULE "message"

/** Creates a new SMFMessage_T object */
SMFMessage_T *smf_message_new(void) {
    CMimeMessage_T *message = cmime_message_new();
    return (SMFMessage_T *)message;
}

/** Free SMFMessage_T object */
void smf_message_free(SMFMessage_T *message) {
    assert(message);
    cmime_message_free((CMimeMessage_T *)message);
}

/** Generates a unique Message-Id. */
char *smf_message_generate_message_id(void) {
    char *s = NULL;
    char *id = NULL;
    int i;
    int pos = 0;

    s = cmime_message_generate_message_id();
    if (s[0] != '<') {
        id = (char *)malloc(strlen(s) + 4);
        id[pos++] = '<';
        for (i = 0; i < strlen(s); i++) {
            id[pos++] = s[i];
        }
        id[pos++] = '>';
        id[pos++] = '\0';
    } else
        id = strdup(s);

    free(s);
    return id;
}

/** Set the sender's name and address on the message object.*/
void smf_message_set_sender(SMFMessage_T *message, const char *sender) {
    assert(message);
    assert(sender);
    cmime_message_set_sender((CMimeMessage_T *)message, sender);
}

/** Gets the email address of the sender from message. */
SMFEmailAddress_T *smf_message_get_sender(SMFMessage_T *message) {
    assert(message);
    return (SMFEmailAddress_T *)cmime_message_get_sender((CMimeMessage_T *)message);
}

char *smf_message_get_sender_string(SMFMessage_T *message) {
    return cmime_message_get_sender_string((CMimeMessage_T *)message);
}

void smf_message_set_message_id(SMFMessage_T *message,const char *message_id) {
    assert(message);
    assert(message_id);
    cmime_message_set_message_id((CMimeMessage_T *)message,message_id);
}

const char *smf_message_get_message_id(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_message_id((CMimeMessage_T *)message);
}


int smf_message_set_header(SMFMessage_T *message, const char *header) {
    assert(message);
    assert(header);

    return cmime_message_set_header((CMimeMessage_T *)message,header);
}


SMFHeader_T *smf_message_get_header(SMFMessage_T *message, const char *header) {
    assert(message);
    assert(header);
    return (SMFHeader_T *)cmime_message_get_header((CMimeMessage_T *)message,header);    
}

/** Add a recipient of a chosen type to the message object. */
int smf_message_add_recipient(SMFMessage_T *message, const char *recipient, SMFEmailAddressType_T t) {
    assert(message);
    assert(recipient);
    return cmime_message_add_recipient((CMimeMessage_T *)message,recipient,(CMimeAddressType_T)t);
}

SMFList_T *smf_message_get_recipients(SMFMessage_T *message) {
    assert(message);
    return (SMFList_T *)cmime_message_get_recipients((CMimeMessage_T *)message);
}

void smf_message_set_content_type(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_content_type((CMimeMessage_T *)message, s);
}

char *smf_message_get_content_type(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_content_type((CMimeMessage_T *)message);
}

void smf_message_set_content_transfer_encoding(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_content_transfer_encoding((CMimeMessage_T *)message,s);
}

char *smf_message_get_content_transfer_encoding(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_content_transfer_encoding((CMimeMessage_T *)message);
}

void smf_message_set_content_id(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_content_id((CMimeMessage_T *)message,s);
}

char *smf_message_get_content_id(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_content_id((CMimeMessage_T *)message);
}

void smf_message_set_mime_version(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_mime_version((CMimeMessage_T *)message,s);
}

char *smf_message_get_mime_version(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_mime_version((CMimeMessage_T *)message);
}

void smf_message_set_date(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_date((CMimeMessage_T *)message, s);
}

char *smf_message_get_date(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_date((CMimeMessage_T *)message);
}

int smf_message_set_date_now(SMFMessage_T *message) {
    assert(message);
    return cmime_message_set_date_now((CMimeMessage_T *)message);
}

void smf_message_set_boundary(SMFMessage_T *message, const char *boundary) {
    assert(message);
    assert(boundary);
    cmime_message_set_boundary((CMimeMessage_T *)message,boundary);
}

char *smf_message_get_boundary(SMFMessage_T *message) {
    assert(message);
    return (char *)cmime_message_get_boundary((CMimeMessage_T *)message);
}

char *smf_message_generate_boundary(void) {
    return cmime_message_generate_boundary();
}

void smf_message_add_generated_boundary(SMFMessage_T *message) {
    assert(message);
    cmime_message_add_generated_boundary((CMimeMessage_T *)message);
}

int smf_message_from_file(SMFMessage_T **message, const char *filename, int header_only) {
    assert(message);
    assert(filename);
    return cmime_message_from_file((CMimeMessage_T **)message,filename,header_only);
}

int smf_message_to_file(SMFMessage_T *message, const char *filename) {
    assert(message);
    assert(filename);
    return cmime_message_to_file((CMimeMessage_T *)message,filename);
}

char *smf_message_to_string(SMFMessage_T *message) {
    assert(message);
    return cmime_message_to_string((CMimeMessage_T *)message);
}

int smf_message_from_string(SMFMessage_T **message, const char *content, int header_only) {
    assert(message);
    assert(content);
    return cmime_message_from_string((CMimeMessage_T **)message,content,header_only);
}

void smf_message_set_subject(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_set_subject((CMimeMessage_T *)message, s);
}

char *smf_message_get_subject(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_subject((CMimeMessage_T *)message);
}

void smf_message_prepend_subject(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_prepend_subject((CMimeMessage_T *)message,s);
}

void smf_message_append_subject(SMFMessage_T *message, const char *s) {
    assert(message);
    assert(s);
    cmime_message_append_subject((CMimeMessage_T *)message,s);
}

int smf_message_set_body(SMFMessage_T *message, const char *content) {
    assert(message);
    assert(content);
    return cmime_message_set_body((CMimeMessage_T *)message,content);
}

int smf_message_append_part(SMFMessage_T *message, SMFPart_T *part) {
    assert(message);
    assert(part);
    return cmime_message_append_part((CMimeMessage_T *)message,(CMimePart_T *)part);
}

int smf_message_get_part_count(SMFMessage_T *message) {
    assert(message);
    return message->parts->size;
}

void smf_message_add_attachment(SMFMessage_T *message, char *attachment) {
    assert(message);
    assert(attachment);
    cmime_message_add_attachment((CMimeMessage_T *)message,attachment);
}

SMFMessage_T *smf_message_create_skeleton(const char *sender, const char *recipient, const char *subject) {
    assert(sender);
    assert(recipient);
    assert(subject);
    return (SMFMessage_T *)cmime_message_create_skeleton(sender,recipient,subject);
}

int smf_message_add_child_part(SMFMessage_T *message, SMFPart_T *part, SMFPart_T *child, SMFMultipartType_T subtype) {
    assert(message);
    assert(part);
    assert(child);
    return smf_message_add_child_part((CMimeMessage_T *)message, (CMimePart_T *)part,(CMimePart_T *)child,(CMimeMultipartType_T)subtype);
}

SMFPart_T *smf_message_part_first(SMFMessage_T *message) {
    SMFPart_T *part = NULL;
    assert(message);
    part = (SMFPart_T *)smf_list_head(message->parts)->data;
    return part;
}

SMFPart_T *smf_message_part_last(SMFMessage_T *message) {
    SMFPart_T *part = NULL;
    assert(message);
    part = (SMFPart_T *)smf_list_tail(message->parts)->data;
    return part;
}
