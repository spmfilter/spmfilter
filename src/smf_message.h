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

#ifndef _SMF_MESSAGE_H
#define	_SMF_MESSAGE_H

#include "smf_core.h"

typedef struct _SMFObject_T SMFMessage_T;
typedef struct _SMFObject_T SMFMimePart_T;
typedef struct _SMFObject_T SMFMultiPart_T;
typedef struct _SMFObject_T SMFDataWrapper_T;

/* struct for messages send
 * via smtp_delivery */
typedef struct {
	/* message sender */
	char *from;

	/* pointer to message recipients */
	char **rcpts;

	/* number of recipients */
	int num_rcpts;

	/* path to message */
	char *message_file;

	/* SMTP auth user, if needed */
	char *auth_user;

	/* SMTP auth password, if needed */
	char *auth_pass;

	/* destination smtp server */
	char *nexthop;

	SMFMessage_T *message;
} SMFMessageEnvelope_T;

/** A message recipient type.
 *
 * SMF_RECIPIENT_TYPE_TO - Represents the recipients in the To: header.
 * SMF_RECIPIENT_TYPE_CC - Represents the recipients in the Cc: header.
 * SMF_RECIPIENT_TYPE_BCC - Represents the recipients in the Bcc: header.
 */
typedef enum {
	SMF_RECIPIENT_TYPE_TO,
	SMF_RECIPIENT_TYPE_CC,
	SMF_RECIPIENT_TYPE_BCC
} SMFRecipientType_T;

/** A Content-Transfer-Encoding enumeration.
 *
 * SMF_CONTENT_ENCODING_DEFAULT - Default transfer encoding.
 * SMF_CONTENT_ENCODING_7BIT - 7bit text transfer encoding.
 * SMF_CONTENT_ENCODING_8BIT - 8bit text transfer encoding.
 * SMF_CONTENT_ENCODING_BINARY - Binary transfer encoding.
 * SNF_CONTENT_ENCODING_BASE64 - Base64 transfer encoding.
 * SMF_CONTENT_ENCODING_QUOTEDPRINTABLE - Quoted-printable transfer encoding.
 * SMF_CONTENT_ENCODING_UUENCODE - Uuencode transfer encoding.
 */
typedef enum {
	SMF_CONTENT_ENCODING_DEFAULT,
	SMF_CONTENT_ENCODING_7BIT,
	SMF_CONTENT_ENCODING_8BIT,
	SMF_CONTENT_ENCODING_BINARY,
	SMF_CONTENT_ENCODING_BASE64,
	SMF_CONTENT_ENCODING_QUOTEDPRINTABLE,
	SMF_CONTENT_ENCODING_UUENCODE,
	SMF_NUM_ENCODINGS
} SMFContentEncoding_T;

/** Creates a new SMFMessageEnvelope_T object
 *
 * \returns an empty SMFMessageEnvelope_T object
 */
SMFMessageEnvelope_T *smf_message_envelope_new(void);

/** Free SMFMessageEnvelope_T object
 *
 * \param message SMFMessageEnvelope_T object
 */
void smf_message_envelope_unref(SMFMessageEnvelope_T *envelope);

/** Add new recipient to envelope
 *
 * \param envelope SMFMessageEnvelope_T object
 * \param rcpt rcpt address
 *
 * \returns SMFMessageEnvelope_T object
 */
SMFMessageEnvelope_T *smf_message_envelope_add_rcpt(SMFMessageEnvelope_T *envelope, const char *rcpt);

/** Deliver message
 *
 * \param msg_data Messate_T structure
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_deliver(SMFMessageEnvelope_T *msg_data);

/** Decodes an rfc2047 encoded 'text' header.
 *
 * \param text header text to decode
 *
 * \returns a newly allocated UTF-8 string representing the the decoded header.
 */
char *smf_message_decode_text(const char *text);

/** Encodes a 'text' header according to the rules in rfc2047.
 *
 * \param text text to encode
 *
 * \returns the encoded header. Useful for encoding headers like "Subject".
 */
char *smf_message_encode_text(const char *text);

/** Generates a unique Message-Id.
 * 
 * \returns a unique string in an addr-spec format suitable for use as a Message-Id.
 */
char *smf_message_generate_message_id(void);

/** Determines the best content encoding for the first len bytes of text.
 *
 * \param text text to encode
 * \param len text length
 *
 * \returns a SMFContentEncoding that is determined to be the best encoding
 *          type for the specified block of text. ("best" in this particular
 *          case means smallest output size)
 */
SMFContentEncoding_T smf_message_best_encoding(unsigned char *text, size_t len);

/** Creates a new SMFMessage_T object
 *
 * \returns an empty message object
 */
SMFMessage_T *smf_message_new(void);

/** Free SMFMessage_T object
 *
 * \param message SMFMessage_T object
 */
void smf_message_unref(SMFMessage_T *message);

/** Set the sender's name and address on the message object.
 * 
 * \param message SMFMessate_T object
 * \param sender The name and address of the sender
 */
void smf_message_set_sender(SMFMessage_T *message, const char *sender);

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
		const char *addr);

/** Set the sender's Reply-To address on the message.
 *
 * \param message SMFMessage to change
 * \param reply_to The Reply-To address
 */
void smf_message_set_reply_to(SMFMessage_T *message, const char *reply_to);

/** Set the unencoded UTF-8 Subject field on a message.
 *
 * \param message SMFMessage_T object
 * \param subject subject string
 */
void smf_message_set_subject(SMFMessage_T *message, const char *subject);

/** Sets the sent-date on a message.
 *
 * \param message SMFMessate_T object
 * \param date Sent-date
 * \param gmt_offset GMT date offset (in +/- hours)
 */
void smf_message_set_date(SMFMessage_T *message, time_t date, int gmt_offset);

/** Set the Message-Id on a message
 *
 * \param message SMFMessage_T object
 * \param message_id the message id
 */
void smf_message_set_message_id(SMFMessage_T *message, const char *message_id);

/** Set the root-level MIME part of the message.
 *
 * \param message SMFMessage_T object
 * \param part SMFMimePart_T object
 */
void smf_message_set_mime_part(SMFMessage_T *message, SMFMimePart_T *part);

/** Set multipart object as root-level MIME part.
 *
 * \param message SMFMessage_T object
 * \param multipart SMFMultiPart_t object
 */
void smf_message_set_multipart(SMFMessage_T *message, SMFMultiPart_T *multipart);

/** Allocates a string buffer containing the contents of SMFMessage_T.
 *
 * \param message a SMFMessage_T object
 *
 * \returns an allocated string containing the contents of SMFMessage_T
 */
char *smf_message_to_string(SMFMessage_T *message);

#endif	/* _SMF_MESSAGE_H */
