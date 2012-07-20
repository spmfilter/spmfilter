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
#include "smf_message.h"

#if 0
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <time.h>

#include "spmfilter_config.h"
#include "smf_email_address.h"
#include "smf_trace.h"

#include "smf_session.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_lookup_private.h"
#endif


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

#if 0
#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"



/** Decodes an rfc2047 encoded 'text' header. */
char *smf_message_decode_text(const char *text) {
    return g_mime_utils_header_decode_text(text);
}

/** Encodes a 'text' header according to the rules in rfc2047. */
char *smf_message_encode_text(const char *text) {
    return g_mime_utils_header_encode_text(text);
}


/** Determines the best content encoding for the first len bytes of text. */
SMFContentEncoding_T smf_message_best_encoding(unsigned char *text, size_t len) {
    return (SMFContentEncoding_T) g_mime_utils_best_encoding(text,len);
}




/** Set the sender's Reply-To address on the message.
 *
 * \param message SMFMessage_T object
 * \param reply_to The Reply-To address
 */
void smf_message_set_reply_to(SMFMessage_T *message, const char *reply_to) {
    g_mime_message_set_reply_to((GMimeMessage *)message->data,reply_to);
}

/** Gets the Reply-To address from message.
 *
 * \param message SMFMessage_T object
 *
 * \returns the sender's Reply-To address from the message.
 */
const char *smf_message_get_reply_to(SMFMessage_T *message) {
    return g_mime_message_get_reply_to((GMimeMessage *)message->data);
}


/** Set the root-level MIME part of the message.
 *
 * \param message SMFMessage_T object
 * \param part SMFMimePart_T object
 */
void smf_message_set_mime_part(SMFMessage_T *message, SMFMimePart_T *part) {
    g_mime_message_set_mime_part((GMimeMessage *)message->data,(GMimeObject *)part->data);
}

/** Set multipart object as root-level MIME part.
 *
 * \param message SMFMessage_T object
 * \param multipart SMFMultiPart_t object
 */
void smf_message_set_multipart(SMFMessage_T *message, SMFMultiPart_T *multipart) {
    g_mime_message_set_mime_part((GMimeMessage *)message->data,(GMimeObject *)multipart->data);
}


/** Recursively calls callback on each of the mime parts in the mime message.
 *
 * \param message SMFMessage_T object
 * \param callback function to call on each of the mime parts contained by the mime message
 * \param user-supplied callback data
 */
void smf_message_foreach(SMFMessage_T *message,
        SMFObjectForeachFunc callback, void  *user_data) {
#ifdef HAVE_GMIME24
    g_mime_message_foreach((GMimeMessage *)message->data,
            (GMimeObjectForeachFunc) callback, user_data);
#else
    g_mime_message_foreach_part((GMimeMessage *)message->data,
            (GMimePartFunc) callback,user_data);
#endif
}

/** Gets the value of the first header with the name requested.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the wanted header
 *
 * \returns value of header or NULL in case of error
 */
const char *smf_message_header_get(SMFMessage_T *message, const char *header_name) {
    return g_mime_object_get_header((GMimeObject *)message->data,header_name);
}

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_prepend(SMFMessage_T *message, char *header_name, char *header_value) {
#ifdef HAVE_GMIME24
    g_mime_object_prepend_header((GMimeObject* )message->data,header_name,header_value);
#else
    g_mime_object_add_header((GMimeObject *)message->data,header_name,header_value);
#endif
}

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_append(SMFMessage_T *message, char *header_name, char *header_value) {
#ifdef HAVE_GMIME24
    g_mime_object_append_header((GMimeObject* )message->data,header_name,header_value);
#else
    g_mime_object_add_header((GMimeObject *)message->data,header_name,header_value);
#endif
}

/** Set the value of the specified header. If value is NULL and the header,
 * name, had not been previously set, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * Note: If there are multiple headers with the specified field name,
 * the first instance of the header will be replaced and further instances
 * will be removed.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_set(SMFMessage_T *message, char *header_name, char *header_value) {
    g_mime_object_set_header((GMimeObject *)message->data,header_name,header_value);
}

/** Removed the specified header if it exists
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 */
void smf_message_header_remove(SMFMessage_T *message, char *header_name) {
    g_mime_object_remove_header((GMimeObject *)message->data,header_name);
}

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \param message a SMFMessage_T object
 *
 * \returns a string containing the header block.
 */
char *smf_message_header_to_string(SMFMessage_T *message) {
    return g_mime_object_get_headers((GMimeObject *)message->data);
}

#endif