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

#ifndef _SMF_SESSION_H
#define	_SMF_SESSION_H

#include "smf_message.h"
#include "smf_lookup.h"

typedef struct {
	char *addr;
	int is_local;
	SMFLookupResult_T *user_data;
} SMFEmailAddress_T;

typedef struct {
	/* hello we received */
	char *helo;

	/* envelope recipients */
	SMFEmailAddress_T **envelope_to;
	int envelope_to_num;

	/* envelope sender */
	SMFEmailAddress_T *envelope_from;

	SMFEmailAddress_T **message_to;
	int message_to_num;

	SMFEmailAddress_T *message_from;

	/* size of message body */
	size_t msgbodysize;

	/* this is our spooling file */
	char *queue_file;

	/* xfoward */
	char *xforward_addr;

	/* message header */
	void *headers;
	void *dirty_headers;
} SMFSession_T;

/** Retrieve SMFSession_T structure
 *
 * \returns pointer to SMFSession_T type
 */
SMFSession_T *smf_session_get(void);

/** Free SMFSession_T structure */
void smf_session_free(void);

/** Copy the current message to disk
 *
 * \param path for the new message file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_to_file(char *path);

/** Gets the value of the first header with the name requested.
 *
 * \param header_name name of the wanted header
 *
 * \returns value of header or NULL in case of error
 */
const char *smf_session_header_get(const char *header_name);

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_prepend(char *header_name, char *header_value);

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_append(char *header_name, char *header_value);

/** Set the value of the specified header. If value is NULL and the header,
 * name, had not been previously set, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * Note: If there are multiple headers with the specified field name,
 * the first instance of the header will be replaced and further instances
 * will be removed.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_set(char *header_name, char *header_value);

/** Removed the specified header if it exists
 *
 * \param header_name name of the header
 */
void smf_session_header_remove(char *header_name);

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \returns a string containing the header block.
 */
char *smf_session_header_to_string(void);

/** Function signature for the callback to smf_session_header_foreach()
 *
 * \param name the field name.
 * \param value the field value.
 * \param user_data the user-supplied callback data.
 */
typedef void (*SMFHeaderForeachFunc) (const char *name, const char *value, void *user_data);

/** Calls func for each header name/value pair.
 *
 * \param func function to be called for each header.
 * \param user_data user data to be passed to the func.
 */
void smf_session_header_foreach(SMFHeaderForeachFunc func, void *user_data);

/** Prepend text to subject
 *
 * \param text text to prepend
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_subject_prepend(char *text);

/** Append text to subject
 *
 * \param text text to append
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_subject_append(char *text);

/** Retrieve a SMFMessage_T object from the
 *  current session.
 *
 * \returns SMFMessage_T object
 */
SMFMessage_T *smf_session_get_message(void);
#endif	/* _SMF_SESSION_H */

