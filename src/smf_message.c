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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <glib.h>
#include <gmime/gmime.h>

#include "spmfilter_config.h"
#include "smf_session.h"
#include "smf_trace.h"
#include "smf_core.h"
#include "smf_message.h"
#include "smf_message_private.h"
#include "smf_lookup.h"
#include "smf_settings.h"

#define THIS_MODULE "message"

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"

/** Creates a new SMFMessageEnvelope_T object
 *
 * \returns an empty SMFMessageEnvelope_T object
 */
SMFMessageEnvelope_T *smf_message_envelope_new(void) {
	SMFMessageEnvelope_T *envelope = NULL;

	envelope = g_slice_new(SMFMessageEnvelope_T);
	envelope->rcpts = NULL;
	envelope->num_rcpts = 0;
	envelope->message_file = NULL;
	envelope->message = NULL;
	envelope->auth_pass = NULL;
	envelope->auth_user = NULL;
	envelope->from = NULL;
	envelope->nexthop = NULL;

	return envelope;
}

/** Free SMFMessageEnvelope_T object
 *
 * \param message SMFMessageEnvelope_T object
 */
void smf_message_envelope_unref(SMFMessageEnvelope_T *envelope) {
	int i;
	/* free all allocated recipient resources */
	for (i = 0; i < envelope->num_rcpts; i++) {
		g_free(envelope->rcpts[i]);
	}

	/* free all other message stuff */
	if (envelope->from != NULL) 
		g_free(envelope->from);
	if (envelope->message_file != NULL)
		g_free(envelope->message_file);
	if (envelope->nexthop != NULL)
		g_free(envelope->nexthop);
	if (envelope->message != NULL)
		smf_message_unref(envelope->message);
	g_slice_free(SMFMessageEnvelope_T,envelope);
}

/** Add new recipient to envelope
 *
 * \param envelope SMFMessageEnvelope_T object
 * \param rcpt rcpt address
 *
 * \returns SMFMessageEnvelope_T object
 */
SMFMessageEnvelope_T *smf_message_envelope_add_rcpt(SMFMessageEnvelope_T *envelope, const char *rcpt) {
	if (envelope->rcpts == NULL) {
		envelope->rcpts = g_malloc(sizeof(envelope->rcpts[1]));
	} else {
		envelope->rcpts = g_realloc(envelope->rcpts,sizeof(envelope->rcpts[envelope->num_rcpts]));
	}
	envelope->rcpts[envelope->num_rcpts] = g_strdup(rcpt);
	envelope->num_rcpts += 1;
	
	return envelope;
}

void smf_message_extract_addresses(GMimeObject *message) {
	SMFSession_T *session = smf_session_get();
	SMFSettings_T *settings = smf_settings_get();
	InternetAddressList *ia;
	InternetAddress *addr;

	/* get the from field */
	session->message_from = g_slice_new(SMFEmailAddress_T);
	session->message_from->addr = smf_core_get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(GMIME_MESSAGE(message)),1);

	TRACE(TRACE_DEBUG,"session->message_from: %s",session->message_from->addr);

	if(settings->backend != NULL) {
		session->message_from->is_local = smf_lookup_check_user(session->message_from->addr);
	}
	TRACE(TRACE_DEBUG,"[%s] is local [%d]",
			session->message_from->addr,
			session->message_from->is_local);

	/* now check the to field */
	session->message_to_num = 0;

	session->message_to = malloc(sizeof(session->message_to[session->message_to_num]));

#ifdef HAVE_GMIME24
	/* g_mime_message_get_all_recipients() appeared in gmime 2.2.5 */
	ia = g_mime_message_get_all_recipients(GMIME_MESSAGE(message));
	if (ia != NULL) {
		int i;
		for (i=0; i < internet_address_list_length(ia); i++) {
			addr = internet_address_list_get_address(ia,i);
			session->message_to[session->message_to_num] = malloc(
					sizeof(*session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			TRACE(TRACE_DEBUG,"session->message_to[%d]: %s",
					session->message_to_num,
					session->message_to[session->message_to_num]->addr);

			if (settings->backend != NULL) {
				session->message_to[session->message_to_num]->is_local =
						smf_lookup_check_user(session->message_to[session->message_to_num]->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]",
						session->message_to[session->message_to_num]->addr,
						session->message_to[session->message_to_num]->is_local);
			}
			session->message_to_num++;
		}
	}
#else
	/* all recipients from to */
	ia = (InternetAddressList *)g_mime_message_get_recipients(
		(GMimeMessage *)message,
		GMIME_RECIPIENT_TYPE_TO
	);
	if (ia != NULL) {

		/* all recipients from cc */
		internet_address_list_concat(
			ia,
			(InternetAddressList *)g_mime_message_get_recipients(
				(GMimeMessage *)message,
				GMIME_RECIPIENT_TYPE_CC
			)
		);

		/* all recipients from bcc */
		internet_address_list_concat(
			ia,
			(InternetAddressList *)g_mime_message_get_recipients(
				(GMimeMessage *)message,
				GMIME_RECIPIENT_TYPE_BCC
			)
		);

		while(ia) {
			addr = internet_address_list_get_address(ia);
			session->message_to = malloc(sizeof(session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num] = malloc(sizeof(*session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			TRACE(TRACE_DEBUG,"session->message_to[%d]: %s",
					session->message_to_num,
					session->message_to[session->message_to_num]->addr);

			if (settings->backend != NULL) {
				session->message_to[session->message_to_num]->is_local =
						smf_lookup_check_user(session->message_to[session->message_to_num]->addr);
				TRACE(TRACE_DEBUG,"[%s] is local [%d]",
						session->message_to[session->message_to_num]->addr,
						session->message_to[session->message_to_num]->is_local);
			}
			session->message_to_num++;
			ia = internet_address_list_next(ia);
		}
	}
#endif

}

/** Decodes an rfc2047 encoded 'text' header.
 *
 * \param text header text to decode
 *
 * \returns a newly allocated UTF-8 string representing the the decoded header.
 */
char *smf_message_decode_text(const char *text) {
	return g_mime_utils_header_decode_text(text);
}

/** Encodes a 'text' header according to the rules in rfc2047.
 *
 * \param text text to encode
 *
 * \returns the encoded header. Useful for encoding headers like "Subject".
 */
char *smf_message_encode_text(const char *text) {
	return g_mime_utils_header_encode_text(text);
}

/** Generates a unique Message-Id.
 *
 * \returns a unique string in an addr-spec format suitable for use as a Message-Id.
 */
char *smf_message_generate_message_id(void) {
	char *mid = NULL;
	char hostname[256];

	gethostname(hostname,256);
	mid = g_strdup(g_mime_utils_generate_message_id(hostname));
	
	return mid;
}

/** Determines the best content encoding for the first len bytes of text.
 *
 * \param text text to encode
 * \param len text length
 *
 * \returns a SMFContentEncoding that is determined to be the best encoding
 *          type for the specified block of text. ("best" in this particular
 *          case means smallest output size)
 */
SMFContentEncoding_T smf_message_best_encoding(unsigned char *text, size_t len) {
	return (SMFContentEncoding_T) g_mime_utils_best_encoding(text,len);
}

/** Creates a new SMFMessage_T object
 *
 * \returns an empty message object
 */
SMFMessage_T *smf_message_new(void) {
	SMFMessage_T *message = NULL;
	char *message_id;
	message = g_slice_new(SMFMessage_T);
	message->data = g_mime_message_new(TRUE);
	message_id = smf_message_generate_message_id();
	smf_message_set_message_id(message,message_id);
	return message;
}

/** Free SMFMessage_T object
 *
 * \param message SMFMessage_T object
 */
void smf_message_unref(SMFMessage_T *message) {
	g_object_unref(message->data);
	g_slice_free(SMFMessage_T,message);
}

/** Set the sender's name and address on the message object.
 *
 * \param message SMFMessate_T object
 * \param sender The name and address of the sender
 */
void smf_message_set_sender(SMFMessage_T *message, const char *sender) {
	g_mime_message_set_sender((GMimeMessage *)message->data,sender);
}

/** Gets the email address of the sender from message.
 *
 * \param message SMFmessage_T object
 *
 * \returns the sender's name and address of the message.
 */
const char *smf_message_get_sender(SMFMessage_T *message) {
	return g_mime_message_get_sender((GMimeMessage *)message->data);
}

/** Add a recipient of a chosen type to the message object.
 *
 * \param message SMFMessate_T object
 * \param type A SMFRecipientType_T
 * \param name The recipient's name (or NULL)
 * \param addr The recipient's address
 */
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
	int fd;
	
	if ((fd = open(filename,O_WRONLY|O_CREAT)) == -1) {
		TRACE(TRACE_ERR,"unable to create %s",filename);
		return -1;
	}

	stream = g_mime_stream_fs_new(fd);
	g_mime_object_write_to_stream((GMimeObject *)message->data,stream);
	
	g_mime_stream_flush(stream);
	g_mime_stream_close(stream);
	g_object_unref(stream);
	close(fd);
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
