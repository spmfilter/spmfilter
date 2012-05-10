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

#ifndef _SMF_SESSION_H
#define _SMF_SESSION_H

#include <assert.h>
#include <glib.h>

#include "smf_envelope.h"
#include "smf_message.h"
#include "smf_trace.h"

/*!
 * @struct SMFSession_T 
 * @brief Holds spmfilter session data
 */
typedef struct {
    SMFEnvelope_T *envelope; /**< message envelope */
    size_t msgbodysize; /**< size of message body */
    char *helo; /**< client's helo */
    char *xforward_addr; /**< xforward data */
    char *response_msg; /**< custom response message */
    int sock_in; /**< in socket */
    int sock_out; /**< out socket */
} SMFSession_T;

/*!
 * @fn void smf_session_set_helo(SMFSession_T *session, char *helo)
 * @brief Set helo for session
 * @param session SMFSession_T object
 * @param helo helo message
 */
void smf_session_set_helo(SMFSession_T *session, char *helo);

/*!
 * @fn char *smf_session_get_helo(SMFSession_T *session)
 * @brief Get session helo
 * @param session SMFSession_T object
 * @returns helo
 */
char *smf_session_get_helo(SMFSession_T *session);

/*!
 * @fn void smf_session_set_xforward_addr(SMFSession_T *session, char *xfwd)
 * @brief Set xforward addr
 * @param session SMFSession_T object
 * @param xfwd xforward address
 */
void smf_session_set_xforward_addr(SMFSession_T *session, char *xfwd);

/*!
 * @fn char *smf_session_get_xforward_addr(SMFSession_T *session)
 * @brief Get xforward address
 * @param session SMFSession_T object
 * @returns xforward address
 */
char *smf_session_get_xforward_addr(SMFSession_T *session);

/*!
 * @fn void smf_session_set_response_msg(SMFSession_T *session, char *rmsg)
 * @brief Set response message
 * @param session SMFSession_T object
 * @param rmsg response message
 */
void smf_session_set_response_msg(SMFSession_T *session, char *rmsg);

/*!
 * @fn char *smf_session_get_response_msg(SMFSession_T *session)
 * @brief Get response message
 * @param session SMFSession_T object
 * @returns response_message
 */
char *smf_session_get_response_msg(SMFSession_T *session);

/*!
 * @fn SMFEnvelope_T *smf_session_get_envelope(SMFSession_T *session)
 * @brief Retrieve the SMFEnvelope_T object from the
 *  current session.
 * @param session a SMFSession_T object
 * @returns SMFEnvelope_T object
 */
SMFEnvelope_T *smf_session_get_envelope(SMFSession_T *session);



#if 0
/** Gets the value of the first header with the name requested.
 *
 * \param header_name name of the wanted header
 *
 * \returns value of header or NULL in case of error
 */
const char *smf_session_header_get(SMFSession_T *session, const char *header_name);

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_prepend(SMFSession_T *session, char *header_name, char *header_value);

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_append(SMFSession_T *session, char *header_name, char *header_value);

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
void smf_session_header_set(SMFSession_T *session, char *header_name, char *header_value);

/** Removed the specified header if it exists
 *
 * \param header_name name of the header
 */
void smf_session_header_remove(SMFSession_T *session, char *header_name);

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \returns a string containing the header block.
 */
char *smf_session_header_to_string(SMFSession_T *session);

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
void smf_session_header_foreach(SMFSession_T *session, SMFHeaderForeachFunc func, void *user_data);

/** Prepend text to subject
 *
 * \param text text to prepend
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_subject_prepend(SMFSession_T *session, char *text);

/** Append text to subject
 *
 * \param text text to append
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_subject_append(SMFSession_T *session, char *text);

#endif

#endif  /* _SMF_SESSION_H */
