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
#include <assert.h>
#include <cmime.h>
 #include <stdio.h>

#include "smf_list.h"
#include "smf_header.h"
#include "smf_internal.h"
#include "smf_message.h"
#include "smf_part.h"
#include "smf_trace.h"


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

char *smf_message_get_message_id(SMFMessage_T *message) {
    assert(message);
    return cmime_message_get_message_id((CMimeMessage_T *)message);
}


int smf_message_set_header(SMFMessage_T *message, const char *header) {
    assert(message);
    assert(header);

    return cmime_message_set_header((CMimeMessage_T *)message,header);
}

int smf_message_update_header(SMFMessage_T *message, const char *header, const char *value) {
    char *header_value;
    int result;

    assert(message);
    assert(header);
    assert(value);

    asprintf(&header_value, "%s: %s", header, value);
    result = cmime_message_set_header(message, header_value);
    free(header_value);
    
    return result;
}

int smf_message_add_header(SMFMessage_T *message, const char *header, const char *value) {
    SMFHeader_T *hdr;

    assert(message);
    assert(header);
    assert(value);

    if ((hdr = smf_message_get_header(message, header)) == NULL) {
        if (smf_message_set_header(message, header) != 0)
            return -1;
        if ((hdr = smf_message_get_header(message, header)) == NULL)
            return -1;
    }

    smf_header_set_value(hdr, value, 0);
    return 0;
}

SMFHeader_T *smf_message_get_header(SMFMessage_T *message, const char *header) {
    assert(message);
    assert(header);
    return (SMFHeader_T *)cmime_message_get_header((CMimeMessage_T *)message,header);    
}

SMFList_T *smf_message_get_headers(SMFMessage_T *message) {
    assert(message);

    return (SMFList_T *)cmime_message_get_headers((CMimeMessage_T *)message);
}

int smf_message_remove_header(SMFMessage_T *message, const char *header_name) {
    SMFListElem_T *elem = NULL;
    SMFHeader_T *header = NULL;
    void *tf = NULL;
    int i = -1;

    elem = smf_list_head(message->headers);
    while(elem != NULL) {
        header = (SMFHeader_T *)smf_list_data(elem);
        if (strcasecmp(header->name,header_name) == 0) {
            i = smf_list_remove(message->headers, elem, &tf);        
            smf_header_free((SMFHeader_T *)tf);
            break;
        }
        elem = elem->next;
    }
    
    return i;
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

int smf_message_to_fd(SMFMessage_T *message, int fd) {
    char *s;
    int total = 0;
    
    assert(message);

    // TODO To decrease the memory footprint you can write the message directly
    //      into the fd (instead of make a detour via the string)
    s = smf_message_to_string(message);
    
    while (total < strlen(s)) {
        int written = write(fd, s + total, strlen(s) - total);
        
        if (written == -1)
            return -1;
        
        total += written;
    }
    
    free(s);
    
    return total;
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

int smf_message_write_skip_header(FILE *src, FILE *dest) {
    int found = 0;
    char *buf = NULL;
    size_t len;
    size_t nwritten = 0;
    
    while (!feof(src)) {
        ssize_t nbytes;
        size_t nbytes_dest;

        if (found == 0) {
            if ((nbytes = getline(&buf, &len, src)) == -1) {
                TRACE(TRACE_ERR, "failed to read queue_file");
                free(buf);
                return -1;
            }

            if ((strcmp(buf, LF) == 0) || (strcmp(buf, CRLF) == 0)) {
                found = 1;
                
                // Prepare "buf" for the following body-read
                free(buf);
                buf = malloc(BUFSIZE);
            }
            
            continue;
        }
        
        if ((nbytes = fread(buf, 1, BUFSIZE, src)) < 0) {
            TRACE(TRACE_ERR, "failed to read queue file: %s (%d)", strerror(errno), errno);
            free(buf);
            return -1;
        }

        nbytes_dest = 0;
        while (nbytes > 0) {
            size_t n;
            if ((n = fwrite(buf + nbytes_dest, 1, nbytes, dest)) == 0) {
                TRACE(TRACE_ERR, "failed to write queue file: %s (%d)", strerror(errno), errno);
                free(buf);
                return -1;
            }
            
            nbytes -= n;
            nbytes_dest += n;
            nwritten += n;
        }
    }
    
    free(buf);

    return nwritten;
}

