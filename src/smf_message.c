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

void smf_message_extract_addresses(GMimeObject *message) {
	SMFSession_T *session = smf_session_get();
	SMFSettings_T *settings = smf_settings_get();
	InternetAddressList *ia;
	InternetAddress *addr;
	int i;

	/* get the from field */
	session->message_from = g_slice_new(SMFEmailAddress_T);
	session->message_from->addr = smf_core_get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(GMIME_MESSAGE(message)),1);

	TRACE(TRACE_DEBUG,"session->message_from: %s",session->message_from->addr);

	session->message_from->is_local = smf_lookup_check_user(session->message_from->addr);
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
	ia = (InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_TO);
	if (ia != NULL) {
		internet_address_list_concat(ia,
			(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_CC));
		internet_address_list_concat(ia,
			(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_BCC));
		while(ia) {
			addr = internet_address_list_get_address(ia);
			session->message_to = malloc(sizeof(session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num] = malloc(sizeof(*session->message_to[session->message_to_num]));
			session->message_to[session->message_to_num]->addr =
					smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
			TRACE(TRACE_DEBUG,"session->message_to[%d]: %s",
					session->message_to_num,
					session->message_to[session->message_to_num]->addr);

			if (strcmp(settings->backend,"undef") != 0) {
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
	mid = g_strdup_printf("<%s>",g_mime_utils_generate_message_id(hostname));
	
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
	message->data = g_mime_message_new(TRUE);
	message_id = smf_message_generate_message_id();
	smf_message_set_message_id(message,message_id);
	return message;
}

/** Set the sender's name and address on the message object.
 *
 * \param message SMFMessate_T object
 * \param sender The name and address of the sender
 */
void smf_message_set_sender(SMFMessage_T *message, const char *sender) {
	g_mime_message_set_sender((GMimeMessage *)message->data,sender);
}

/** Add a recipient of a chosen type to the message object.
 *
 * \param message SMFMessate_T object
 * \param type A SMFRecipientType_T
 * \param name The recipient's name (or NULL)
 * \param addr The recipient's address
 */
void smf_mesage_add_recipient(SMFMessage_T *message,
		SMFRecipientType_T type,
		const char *name,
		const char *addr) {
	g_mime_message_add_recipient((GMimeMessage *)message->data,
			type, name, addr);
}

/** Set the sender's Reply-To address on the message.
 *
 * \param message SMFMessage_T object
 * \param reply_to The Reply-To address
 */
void smf_message_set_reply_to(SMFMessage_T *message, const char *reply_to) {
	g_mime_message_set_reply_to((GMimeMessage *)message->data,reply_to);
}

/** Set the unencoded UTF-8 Subject field on a message.
 *
 * \param message SMFMessage_T object
 * \param subject subject string
 */
void smf_message_set_subject(SMFMessage_T *message, const char *subject) {
	g_mime_message_set_subject((GMimeMessage *)message->data,subject);
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

/** Set the Message-Id on a message
 *
 * \param message SMFMessage_T object
 * \param message_id the message id
 */
void smf_message_set_message_id(SMFMessage_T *message,const char *message_id) {
	g_mime_message_set_message_id((GMimeMessage *)message->data,message_id);
}
