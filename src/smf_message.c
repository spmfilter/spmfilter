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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <gmime/gmime.h>

#include "spmfilter_config.h"
#include "smf_email_address.h"
#include "smf_session.h"
#include "smf_trace.h"
#include "smf_core.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_settings.h"

#define THIS_MODULE "message"

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"

/** Creates a new SMFMessageEnvelope_T object */
SMFMessageEnvelope_T *smf_message_envelope_new(void) {
	SMFMessageEnvelope_T *envelope = NULL;

	envelope = g_slice_new(SMFMessageEnvelope_T);
	envelope->num_rcpts = 0;
	envelope->rcpt = NULL;
	envelope->sender = NULL;
	envelope->message_file = NULL;
	envelope->message = NULL;
	envelope->auth_pass = NULL;
	envelope->auth_user = NULL;
	envelope->nexthop = NULL;
#if 0
	envelope->headers = (void *)g_mime_header_list_new();
	envelope->dirty_headers = NULL; 
#endif
	return envelope;
}

/** Free SMFMessageEnvelope_T object */
void smf_message_envelope_free(SMFMessageEnvelope_T *envelope) {
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

	if (envelope->nexthop != NULL)
		g_free(envelope->nexthop);

	if (envelope->message != NULL)
		smf_message_unref(envelope->message);
		
	if (envelope->message_file != NULL)
		g_free(envelope->message_file);
		
	if (envelope->auth_user != NULL)
		g_free(envelope->auth_user);
		
	if (envelope->auth_pass != NULL)
		g_free(envelope->auth_pass);
		
	g_slice_free(SMFMessageEnvelope_T,envelope);
}

/** Add new recipient to envelope */
SMFMessageEnvelope_T *smf_message_envelope_add_rcpt(SMFMessageEnvelope_T *envelope, char *rcpt) {

	envelope->rcpt = g_realloc(
		envelope->rcpt,
		sizeof(SMFEmailAddress_T) * (envelope->num_rcpts + 1)
	);

	envelope->rcpt[envelope->num_rcpts] = smf_email_address_new();
	envelope->rcpt[envelope->num_rcpts]->addr = g_strdup(rcpt);
	envelope->num_rcpts++;

	return envelope;
}

void smf_message_envelope_foreach_rcpt(SMFMessageEnvelope_T *envelope, 
		SMFRcptForeachFunc callback, void  *user_data) {
	int i;
	
	if (envelope->rcpt != NULL) {
		for (i = 0; i < envelope->num_rcpts; i++) {
			(*callback)(envelope->rcpt[i],user_data);
		}
	}		
}

/** Set envelope sender */
SMFMessageEnvelope_T *smf_message_envelope_set_sender(SMFMessageEnvelope_T *envelope, char *sender) {
	// free sender, if already set...
	if (envelope->sender != NULL)
		smf_email_address_free(envelope->sender);
	
	envelope->sender = smf_email_address_new();
	envelope->sender->addr = g_strdup(sender);
	
	return envelope;
}

/** Get envelope sender */
SMFEmailAddress_T *smf_message_envelope_get_sender(SMFMessageEnvelope_T *envelope) {
	return envelope->sender;
}

/** Set message file path */
SMFMessageEnvelope_T *smf_message_envelope_set_message_file(SMFMessageEnvelope_T *envelope, char *fp) {
	if (envelope->message_file != NULL) {
		g_free(envelope->message_file);
	}
	
	envelope->message_file = g_strdup(fp);
	return envelope;
}

char *smf_message_envelope_get_message_file(SMFMessageEnvelope_T *envelope) {
	return (char *)envelope->message_file;
}

/** Set auth user */
SMFMessageEnvelope_T *smf_message_envelope_set_auth_user(SMFMessageEnvelope_T *envelope, char *auth_user) {
	if (envelope->auth_user != NULL) {
		g_free(envelope->auth_user);
	}
	
	envelope->auth_user = g_strdup(auth_user);
	return envelope;
}

/** Get auth user */
char *smf_message_envelope_get_auth_user(SMFMessageEnvelope_T *envelope) {
	return (char *)envelope->auth_user;
}

/** Set auth password */
SMFMessageEnvelope_T *smf_message_envelope_set_auth_pass(SMFMessageEnvelope_T *envelope, char *auth_pass) {
	if (envelope->auth_pass != NULL) {
		g_free(envelope->auth_pass);
	}
	
	envelope->auth_pass = g_strdup(auth_pass);
	return envelope;
}

/** Get auth pass */
char *smf_message_envelope_get_auth_pass(SMFMessageEnvelope_T *envelope) {
	return (char *)envelope->auth_pass;
}

/** Set nexthop */
SMFMessageEnvelope_T *smf_message_envelope_set_nexthop(SMFMessageEnvelope_T *envelope, char *nexthop) {
	if (envelope->nexthop != NULL) {
		g_free(envelope->nexthop);
	}
	
	envelope->nexthop = g_strdup(nexthop);
	return envelope;
}

/** Get nexthop */
char *smf_message_envelope_get_nexthop(SMFMessageEnvelope_T *envelope) {
	return (char *)envelope->nexthop;
}

void smf_message_extract_addresses(SMFMessageEnvelope_T **envelope) {
	InternetAddressList *ia;
	InternetAddress *addr;
	SMFSettings_T *settings = smf_settings_get();
	SMFMessageEnvelope_T *e = (*envelope);
	
	if (e == NULL) 
		return;
	
	/* get the from field */
	e->message->message_from = g_slice_new(SMFEmailAddress_T);
	e->message->message_from->addr = smf_core_get_substring(
		EMAIL_EXTRACT,g_mime_message_get_sender(GMIME_MESSAGE(e->message)),1);
	e->message->message_from->lr = NULL;
	
	if (e->message->message_from->addr != NULL) {
		TRACE(TRACE_DEBUG,"envelope->message_from: %s",e->message->message_from->addr);
		if (settings->backend != NULL) {
			smf_lookup_check_user(e->message->message_from);
		} else {
			e->message->message_from->lr = NULL;
		}
		TRACE(TRACE_DEBUG,"[%s] is local [%d]",
			e->message->message_from->addr,
			e->message->message_from->is_local);
	}

	/* now check the to field */
	e->message->message_to_num = 0;
	
#ifdef HAVE_GMIME24
	/* g_mime_message_get_all_recipients() appeared in gmime 2.2.5 */
	ia = g_mime_message_get_all_recipients(GMIME_MESSAGE(e->message));
	if (ia != NULL) {
		int i;
		for (i=0; i < internet_address_list_length(ia); i++) {
			addr = internet_address_list_get_address(ia,i);
			e->message->message_to = g_realloc(
					e->message->message_to,
					sizeof(SMFEmailAddress_T) * (e->message->message_to_num + 1)
				);
			e->message->message_to[e->message->message_to_num] = g_slice_new(SMFEmailAddress_T);
			e->message->message_to[e->message->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			e->message->message_to[e->message->message_to_num]->lr = NULL;
			if (e->message->message_to[e->message->message_to_num]->addr != NULL) {
				TRACE(TRACE_DEBUG,"envelope->message_to[%d]: %s",
						e->message->message_to_num,
						e->message->message_to[e->message->message_to_num]->addr);

				if (settings->backend != NULL) {
					e->message->message_to[e->message->message_to_num]->lr = NULL;
					if (e->message->message_to[e->message->message_to_num]->addr != NULL) {
						smf_lookup_check_user(e->message->message_to[e->message->message_to_num]);
						TRACE(TRACE_DEBUG,"[%s] is local [%d]",
							e->message->message_to[e->message->message_to_num]->addr,
							e->message->message_to[e->message->message_to_num]->is_local);
					}
				} else {
					e->message->message_to[e->message->message_to_num]->lr = NULL;
				}
				e->message->message_to_num++;
			}
		}
	}
#else
	/* all recipients from to */
	ia = (InternetAddressList *)g_mime_message_get_recipients(
		(GMimeMessage *)e->message,
		GMIME_RECIPIENT_TYPE_TO
	);
	if (ia != NULL) {

		/* all recipients from cc */
		internet_address_list_concat(
			ia,
			(InternetAddressList *)g_mime_message_get_recipients(
				(GMimeMessage *)e->message,
				GMIME_RECIPIENT_TYPE_CC
			)
		);

		/* all recipients from bcc */
		internet_address_list_concat(
			ia,
			(InternetAddressList *)g_mime_message_get_recipients(
				(GMimeMessage *)e->message,
				GMIME_RECIPIENT_TYPE_BCC
			)
		);

		while(ia) {
			addr = internet_address_list_get_address(ia);

			e->message_to = g_realloc(
					e->message_to,
					sizeof(SMFEmailAddress_T) * (e->message_to_num + 1)
				);
			e->message_to[e->message_to_num] = g_slice_new(SMFEmailAddress_T);
			e->message_to[e->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			e->message_to[e->message_to_num]->lr = NULL;
			if (e->message_to[e->message_to_num]->addr != NULL) {
				TRACE(TRACE_DEBUG,"envelope->message_to[%d]: %s",
						e->message_to_num,
						e->message_to[e->message_to_num]->addr);

				if (settings->backend != NULL) {
					e->message_to[e->message_to_num]->lr = NULL;
					smf_lookup_check_user(e->message_to[e->message_to_num]);
					TRACE(TRACE_DEBUG,"[%s] is local [%d]",
							e->message_to[e->message_to_num]->addr,
							e->message_to[e->message_to_num]->is_local);
				} else {
					e->message_to[e->message_to_num]->lr = NULL;
				}
				e->message_to_num++;
			}
			ia = internet_address_list_next(ia);
		}
	}
#endif

}

/** Decodes an rfc2047 encoded 'text' header. */
char *smf_message_decode_text(const char *text) {
	return g_mime_utils_header_decode_text(text);
}

/** Encodes a 'text' header according to the rules in rfc2047. */
char *smf_message_encode_text(const char *text) {
	return g_mime_utils_header_encode_text(text);
}

/** Generates a unique Message-Id. */
char *smf_message_generate_message_id(void) {
	char *mid = NULL;
	char hostname[256];

	gethostname(hostname,256);
	mid = g_strdup(g_mime_utils_generate_message_id(hostname));
	
	return mid;
}

/** Determines the best content encoding for the first len bytes of text. */
SMFContentEncoding_T smf_message_best_encoding(unsigned char *text, size_t len) {
	return (SMFContentEncoding_T) g_mime_utils_best_encoding(text,len);
}

/** Creates a new SMFMessage_T object */
SMFMessage_T *smf_message_new(void) {
	SMFMessage_T *message = NULL;
	char *message_id;
	message = g_slice_new(SMFMessage_T);
	message->data = g_mime_message_new(TRUE);
	message_id = smf_message_generate_message_id();
	smf_message_set_message_id(message,message_id);
	return message;
}

/** Free SMFMessage_T object */
void smf_message_unref(SMFMessage_T *message) {
	g_object_unref(message->data);
	g_slice_free(SMFMessage_T,message);
}

/** Set the sender's name and address on the message object.*/
void smf_message_set_sender(SMFMessage_T *message, const char *sender) {
	g_mime_message_set_sender((GMimeMessage *)message->data,sender);
}

/** Gets the email address of the sender from message. */
const char *smf_message_get_sender(SMFMessage_T *message) {
	return g_mime_message_get_sender((GMimeMessage *)message->data);
}

/** Add a recipient of a chosen type to the message object. */
void smf_message_add_recipient(SMFMessage_T *message,
		SMFRecipientType_T type,
		const char *name,
		const char *addr)
{
#ifdef HAVE_GMIME24
	g_mime_message_add_recipient(
		(GMimeMessage *)message->data,
		(GMimeRecipientType)type,
		name, addr
	);
#else
	GMimeMessage *msg = (GMimeMessage *)message->data;

	switch(type) {
		case SMF_RECIPIENT_TYPE_TO:
			g_mime_message_add_recipient(msg, GMIME_RECIPIENT_TYPE_TO, name, addr);
			break;
		case SMF_RECIPIENT_TYPE_CC:
			g_mime_message_add_recipient(msg, GMIME_RECIPIENT_TYPE_CC, name, addr);
			break;
		case SMF_RECIPIENT_TYPE_BCC:
			g_mime_message_add_recipient(msg, GMIME_RECIPIENT_TYPE_CC, name, addr);
			break;
	}
#endif
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

/** Set the unencoded UTF-8 Subject field on a message.
 *
 * \param message SMFMessage_T object
 * \param subject subject string
 */
void smf_message_set_subject(SMFMessage_T *message, const char *subject) {
	g_mime_message_set_subject((GMimeMessage *)message->data,subject);
}

/** Gets the subject from message
 *
 * \param message SMFMessage_T object
 *
 * \returns the message subject
 */
const char *smf_message_get_subject(SMFMessage_T *message) {
	return g_mime_message_get_subject((GMimeMessage *)message->data);
}

/** Sets the sent-date on a message.
 *
 * \param message SMFMessate_T object
 * \param date Sent-date
 * \param gmt_offset GMT date offset (in +/- hours)
 */
void smf_message_set_date(SMFMessage_T *message, time_t date, int gmt_offset) {
	g_mime_message_set_date((GMimeMessage *)message->data,date,gmt_offset);
}

/** Stores the date in time_t format in date. If tz_offset is non-NULL, then
 *  the timezone offset in will be stored in tz_offset.
 *
 * \param message SMFMessage_T object
 * \param date pointer to a date in time_t
 * \param tz_offset pointer to timezone offset (in +/- hours)
 */
void smf_message_get_date(SMFMessage_T *message, time_t *date, int *tz_offset) {
	g_mime_message_get_date((GMimeMessage *)message->data,date,tz_offset);
}

/** Set the Message-Id on a message
 *
 * \param message SMFMessage_T object
 * \param message_id the message id
 */
void smf_message_set_message_id(SMFMessage_T *message,const char *message_id) {
	g_mime_message_set_message_id((GMimeMessage *)message->data,message_id);
}

/** Get the Message-Id of a message
 *
 * \param message SMFMessage_T object
 *
 * \returns the message id
 */
const char *smf_message_get_message_id(SMFMessage_T *message) {
	return g_mime_message_get_message_id((GMimeMessage *)message->data);
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

/** Allocates a string buffer containing the contents of SMFMessage_T.
 *
 * \param message a SMFMessage_T object
 *
 * \returns an allocated string containing the contents of SMFMessage_T
 */
char *smf_message_to_string(SMFMessage_T *message) {
	return g_mime_object_to_string((GMimeObject *)message->data);
}

/** Write SMFMessage_T object to disk.
 *
 * \param message SMFMessage_T object
 * \param filename destination file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_to_file(SMFMessage_T *message, const char *filename) {
	GMimeStream *stream;
	FILE *fd;
	
	if ((fd = fopen(filename,"wb+")) == NULL) {
		TRACE(TRACE_ERR,"unable to create %s",filename);
		return -1;
	}

	stream = g_mime_stream_file_new(fd);
	g_mime_object_write_to_stream((GMimeObject *)message->data,stream);
	
	g_mime_stream_flush(stream);
	g_object_unref(stream);
	return 0;
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
