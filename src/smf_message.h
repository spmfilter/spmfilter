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

#ifndef _SMF_MESSAGE_H
#define _SMF_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmime.h>

#include "smf_core.h"
#include "smf_email_address.h"
#include "smf_header.h"
#include "smf_list.h"
#include "smf_part.h"

#if 0 
#include "smf_lookup.h"

#include "smf_settings.h"
#endif 

/*!
 * @struct SMFMessage_T smf_message.h
 * @brief Represents an email message
 */

typedef CMimeMessage_T SMFMessage_T;

/*!
 * @fn SMFMessage_T *smf_message_new(void)
 * @brief Creates a new SMFMessage_T object
 * @returns an empty message object
 */
SMFMessage_T *smf_message_new(void);

/*!
 * @fn void smf_message_free(SMFMessage_T *message)
 * @brief Free SMFMessage_T object
 * @param message SMFMessage_T object
 */
void smf_message_free(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_sender(SMFMessage_T *message, const char *sender)
 * @brief Set the sender's name and address on the message object.
 * @param message SMFMessage_T object
 * @param sender The name and address of the sender
 */
void smf_message_set_sender(SMFMessage_T *message, const char *sender);

/*!
 * @fn SMFEmailAddress_T *smf_message_get_sender(SMFMessage_T *message)
 * @brief Gets a SMFEmailAddress_T object of the sender from message.
 * @param message SMFmessage_T object
 * @returns the sender's name and address of the message.
 */
SMFEmailAddress_T *smf_message_get_sender(SMFMessage_T *message);

/*!
 * @fn char *smf_message_get_sender_string(SMFMessage_T *message)
 * @brief Get sender of a SMFMessage_T object as string
 * @param message a SMFMessage_T object
 * @returns sender of message as newly allocated string
 */
char *smf_message_get_sender_string(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_message_id(SMFMessage_T *message, const char *message_id)
 * @brief Set the Message-Id on a message
 * @param message SMFMessage_T object
 * @param message_id the message id
 */
void smf_message_set_message_id(SMFMessage_T *message, const char *message_id);

/*!
 * @fn const char *smf_message_get_message_id(SMFMessage_T *message)
 * @brief Get the Message-Id of a message
 * @param message SMFMessage_T object
 * @returns the message id
 */
const char *smf_message_get_message_id(SMFMessage_T *message);

/*!
 * @fn char *smf_message_generate_message_id(void)
 * @brief Generates a unique Message-Id.
 * @returns a unique string in an addr-spec format suitable for use as a Message-Id.
 */
char *smf_message_generate_message_id(void);

/*!
 * @fn int smf_message_set_header(SMFMessage_T *message, const char *header)
 * @brief Set a header to message object. If header already exists, it will be overwritten
 * @param message a SMFMessage_T object
 * @param header full header string
 * @returns 0 on success or -1 in case of error
 */
int smf_message_set_header(SMFMessage_T *message, const char *header);

/*!
 * @fn SMFHeader_T *smf_message_get_header(SMFMessage_T *message, const char *header)
 * @brief Get header for given key
 * @param message a SMFMessage_T object
 * @param header name of header to search for
 * @returns a SMFHeader_T object, or NULL in case of error
 */
SMFHeader_T *smf_message_get_header(SMFMessage_T *message, const char *header);

/*!
 * @fn int smf_message_add_recipient(SMFMessage_T *message, const char *recipient, SMFEmailAddressType_T t)
 * @brief Add a recipient of a chosen type to the message object.
 * @param message SMFMessage_T object
 * @param recipient recipient string
 * @param t A SMFEmailAddressType_T
 * @returns 0 on success or -1 in case of error
 */
int smf_message_add_recipient(SMFMessage_T *message, const char *recipient, SMFEmailAddressType_T t);

/*!
 * @fn SMFList_T *smf_message_get_recipients(SMFMessage_T *message)      
 * @brief Get list with all recipients of message
 * @param message a SMFMessage_T object
 * @returns SMFList_T with recipients
 */
SMFList_T *smf_message_get_recipients(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_content_type(SMFMessage_T *message, const char *t)
 * @brief Set Content-Type header
 * @param message a SMFMessage_T object
 * @param s Content-Type string
 */
void smf_message_set_content_type(SMFMessage_T *message, const char *s);

/*!
 * @fn char *smf_message_get_content_type(SMFMessage_T *message)
 * @brief Get Content-Type header value
 * @param message a SMFMessage_T object
 * @returns Content-Type header value
 */
char *smf_message_get_content_type(SMFMessage_T *message);

/*! 
 * @fn void smf_message_set_content_transfer_encoding(SMFMessage_T *message, const char *s)
 * @brief set Content-Transfer-Encoding value
 * @param message a SMFMessage_T object
 * @param s Content-Transfer-Encoding value
 */
void smf_message_set_content_transfer_encoding(SMFMessage_T *message, const char *s);

/*!
 * @fn char *smf_message_get_content_transfer_encoding(SMFMessage_T *message)
 * @brief Get Content-Transfer-Encoding value
 * @param message a SMFMessage_T object
 * @returns Content-Transfer-Encoding header value
 */
char *smf_message_get_content_transfer_encoding(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_content_id(SMFMessage_T *message, const char *s)
 * @brief Set Content-ID header value
 * @param message a SMFMessage_T object
 * @param s Content-ID header value
 */
void smf_message_set_content_id(SMFMessage_T *message, const char *s);

/*!
 * @fn char *smf_message_get_content_id(SMFMessage_T *message)
 * @brief Get Content-ID header value
 * @param message a SMFMessage_T object
 * @returns Content-ID header value
 */
char *smf_message_get_content_id(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_mime_version(SMFMessage_T *message, const char *v)
 * @brief Set Mime-Version header value. According to RFC 2045, Mime-Version header 
 *   is required at the top level of a message. Not required for each body part of 
 *   a multipart entity. It's required for the embedded headers of a body of type 
 *   "message/rfc822" or "message/partial" if the embedded message is itself 
 *   claimed to be MIME.
 * @param message a SMFMessage_T object
 * @param s Mime-Version header value
 */
void smf_message_set_mime_version(SMFMessage_T *message, const char *s);

/*!
 * @fn char *smf_message_get_mime_version(SMFMessage_T *message)
 * @brief Get Mime-Version header value
 * @param message a SMFMessage_T object
 * @returns Mime-Version header value
 */
char *smf_message_get_mime_version(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_date(SMFMessage_T *message, const char *s)
 * @brief Set date string header 
 * @param message a SMFMessage_T object
 * @param s date string
 */
void smf_message_set_date(SMFMessage_T *message, const char *s);

/*!
 * @fn char *smf_message_get_date(SMFMessage_T *message)
 * @brief Get date string from SMFMessage_T object
 * @param message a SMFMessage_T object
 * @returns date string
 */
char *smf_message_get_date(SMFMessage_T *message);

/*!
 * @fn int smf_message_set_date_now(SMFMessage_T *message)
 * @brief Get current time and set date header
 * @param message a SMFMessage_T object
 * @returns 0 on success or -1 in case of error
 */
int smf_message_set_date_now(SMFMessage_T *message);

/*!
 * @fn void smf_message_set_boundary(SMFMessage_T *message, const char *boundary)
 * @brief Set message boundary
 * @param message a SMFMessage_T object
 * @param boundary the boundary to set
 */
void smf_message_set_boundary(SMFMessage_T *message, const char *boundary);

/*! 
 * @fn char *smf_message_get_boundary(SMFMessage_T *message)
 * @brief Get message boundary
 * @param message a SMFMessage_T object
 * @returns message boundary
 */
char *smf_message_get_boundary(SMFMessage_T *message);

/*! 
 * @fn char *smf_message_generate_boundary(void)
 * @brief Generate a message boundary
 * @returns a newly allocated boundary
 */
char *smf_message_generate_boundary(void);

/*!
 * @fn void smf_message_add_generated_boundary(SMFMessage_T *message)
 * @brief Add a newly generated boundary to a SMFMessage_T object
 * @param message a SMFMessage_T object
 */
void smf_message_add_generated_boundary(SMFMessage_T *message);

/*!
 * @fn void SMF_message_set_subject(SMFMessage_T *message, const char *subject)
 * @brief Set message subject
 * @param message a SMFMessage_T object
 * @param s the subject string
 */
void smf_message_set_subject(SMFMessage_T *message, const char *s);

/*!
 * @fn char *SMF_message_get_subject(SMFMessage_T *message)
 * @brief Get the subject string from SMFMessage_T object
 * @param message a SMFMessage_T object
 * @returns subject string to append
 */
char *smf_message_get_subject(SMFMessage_T *message);

/*!
 * @fn int smf_message_from_file(SMFMessage_T **message, const char *filename, int header_only)
 * @brief Parse given file and create a SMFMessage_T object
 * @param message out param to return the new message object
 * @param filename path to message file
 * @param header_only parse only message headers, 1 = tue, 0 = false
 * @returns 0 on success, -1 on stat error, -2 if not a regular file, -3 if reading fails, 
 *   1 if parsing failed because of invalid input, 2 if parsing failed due to memory exhaustion
 */
int smf_message_from_file(SMFMessage_T **message, const char *filename, int header_only);

/*!
 * @fn int smf_message_to_file(SMFMessage_T *message, const char *filename)
 * @brief Write SMFMessage_T object to file
 * @param message a SMFMessage_T object
 * @param filename path to file, which should be written
 * @returns the number of items successfully written on success, -1 in case of error
 */
int smf_message_to_file(SMFMessage_T *message, const char *filename);

/*!
 * @fn char *smf_message_to_string(SMFMessage_T *message) 
 * @brief Allocates a string buffer containing the contents of SMFMessage_T.
 * @param message a SMFMessage_T object 
 * @returns message as newly allocated string
 */
char *smf_message_to_string(SMFMessage_T *message);

/*!
 * @fn int smf_message_from_string(SMFMessage_T **message, const char *content, int header_only)
 * @brief Parse given string and create a new SMFMessage_T object
 * @param message out param to return the new message object
 * @param content message string to parse
 * @param header_only parse only message headers, 1 = tue, 0 = false
 * @returns 0 on success or -1 in case of error, 1 if parsing failed because of invalid input, 
 *   2 if parsing failed due to memory exhaustion
 */
int smf_message_from_string(SMFMessage_T **message, const char *content, int header_only);

/*!
 * @fn void smf_message_prepend_subject(SMFMessage_T *message, const char *s)
 * @brief prepend string to subject
 * @param message a SMFMessage_T object
 * @param s string to prepend
 */
void smf_message_prepend_subject(SMFMessage_T *message, const char *s);

/*!
 * @fn void smf_message_append_subject(SMFMessage_T *message, const char *s)
 * @brief append string to subject
 * @param message a SMFMessage_T object
 * @param s a append string for subject
 */
void smf_message_append_subject(SMFMessage_T *message, const char *s);

/*!
 * @fn int smf_message_set_body(SMFMessage_T *message, const char *body)
 * @brief Set plain body to non multipart message.
 * @param message a SMFMessage_T object
 * @param content body content
 * @returns 0 on sucess, -1 if message is multipart
 */
int smf_message_set_body(SMFMessage_T *message, const char *content);

/*!
 * @fn int smf_message_append_part(SMFMessage_T *message, SMFPart_T *part)
 * @brief Append mime part to message object and generate boundary if necessary
 * @param message a SMFMessage_T object
 * @param part a SMFPart_T part
 * @returns 0 on success, -1 in case of error
 */
int smf_message_append_part(SMFMessage_T *message, SMFPart_T *part);

#if 0
typedef struct _SMFObject_T SMFObject_T;
typedef struct _SMFObject_T SMFMimePart_T;
typedef struct _SMFObject_T SMFMultiPart_T;
typedef struct _SMFObject_T SMFDataWrapper_T;

typedef struct {
    SMFEmailAddress_T **message_to;
    int message_to_num;
 
    SMFEmailAddress_T *message_from;
    
    void *headers;
    void *dirty_headers;
    
    void *data;
} SMFMessage_T;

 /*!
 * @enum SMFRecipientType_T
 * @brief Possible recipient types
 */
typedef enum {
    SMF_RECIPIENT_TYPE_TO, /**< Represents the recipients in the To: header */
    SMF_RECIPIENT_TYPE_CC, /**< Represents the recipients in the Cc: header */
    SMF_RECIPIENT_TYPE_BCC /**< Represents the recipients in the Bcc: header */
} SMFRecipientType_T;

/** A Content-Transfer-Encoding enumeration.
 *
 * SMF_CONTENT_ENCODING_DEFAULT - Default transfer encoding.
 * SMF_CONTENT_ENCODING_7BIT - 7bit text transfer encoding.
 * SMF_CONTENT_ENCODING_8BIT - 8bit text transfer encoding.
 * SMF_CONTENT_ENCODING_BINARY - Binary transfer encoding.
 * SMF_CONTENT_ENCODING_BASE64 - Base64 transfer encoding.
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

/** Deliver message
 *
 * \param msg_data SMFMessageEnvelope_T object
 *
 * \returns 0 on success or -1 in case of error
 */
//int smf_message_deliver(SMFMessageEnvelope_T *envelope);

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



/** Determines the best content encoding for the first len bytes of text.
 *
 * \param text text to encode
 * \param len text length
 *
 * \returns a SMFContentEncoding_T that is determined to be the best encoding
 *          type for the specified block of text. ("best" in this particular
 *          case means smallest output size)
 */
SMFContentEncoding_T smf_message_best_encoding(unsigned char *text, size_t len);

/** Free SMFMessage_T object
 *
 * \param message SMFMessage_T object
 */
void smf_message_unref(SMFMessage_T *message);







/** Set the sender's Reply-To address on the message.
 *
 * \param message SMFMessage to change
 * \param reply_to The Reply-To address
 */
void smf_message_set_reply_to(SMFMessage_T *message, const char *reply_to);

/** Gets the Reply-To address from message.
 *
 * \param message SMFMessage_T object
 *
 * \returns the sender's Reply-To address from the message.
 */
const char *smf_message_get_reply_to(SMFMessage_T *message);


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


#ifdef HAVE_GMIME24
/** The function signature for a callback to smf_message_foreach() and smf_mime_multipart_foreach().
 *
 * \param parent parent SMFObject_T
 * \param part a SMFObject_T
 * \param user_data User-supplied callback data.
 */
typedef void (*SMFObjectForeachFunc) (SMFObject_T *parent, SMFObject_T *part, void *user_data);
#else
/** The function signature for a callback to smf_message_foreach() and smf_mime_multipart_foreach().
 *
 * \param part a SMFObject_T
 * \param user_data User-supplied callback data.
 */
typedef void (*SMFObjectForeachFunc) (SMFObject_T *part, void *user_data);
#endif

/** Recursively calls callback on each of the mime parts in the mime message.
 *
 * \param message SMFMessage_T object
 * \param callback function to call on each of the mime parts contained by the mime message
 * \param user-supplied callback data
 */
void smf_message_foreach(SMFMessage_T *message, SMFObjectForeachFunc callback, void  *user_data);

/** Gets the value of the first header with the name requested.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the wanted header
 *
 * \returns value of header or NULL in case of error
 */
const char *smf_message_header_get(SMFMessage_T *message, const char *header_name);

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_prepend(SMFMessage_T *message, char *header_name, char *header_value);

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_append(SMFMessage_T *message, char *header_name, char *header_value);

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
void smf_message_header_set(SMFMessage_T *message, char *header_name, char *header_value);

/** Removed the specified header if it exists
 *
 * \param message a SMFMessage_T object
 * \param header_name name of the header
 */
void smf_message_header_remove(SMFMessage_T *message, char *header_name);

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \param message a SMFMessage_T object
 *
 * \returns a string containing the header block.
 */
char *smf_message_header_to_string(SMFMessage_T *message);

#endif

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_MESSAGE_H */
