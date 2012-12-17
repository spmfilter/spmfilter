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

#include "smf_envelope.h"

/*!
 * @struct SMFSession_T 
 * @brief Holds spmfilter session data
 */
typedef struct {
    SMFEnvelope_T *envelope; /**< message envelope */
    size_t message_size; /**< size of message body */
    char *message_file; /**< path to message */
    char *helo; /**< client's helo */
    char *xforward_addr; /**< xforward data */
    char *response_msg; /**< custom response message */
    int sock; /**< socket */
    char *id; /**< session id **/
} SMFSession_T;

/*!
 * @fn SMFSession_T *smf_session_new(void)
 * @brief Create a new SMFSession_T object
 * @returns a newly allocated SMFSession_T object
 */
SMFSession_T *smf_session_new(void);

/*!
 * @fn void smf_session_free(SMFSession_T *session)
 * @brief Free a SMFSession_T object
 * @param session a SMFSession_T object
 */
void smf_session_free(SMFSession_T *session);

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

/*!
 * @fn void smf_envelope_set_message_file(SMFSession_T *session, char *fp)
 * @brief Set path for message file
 * @param session SMFSession_T object
 * @param fp message file path
 */
void smf_session_set_message_file(SMFSession_T *session, char *fp);

/*!
 * @fn char *smf_envelope_get_message_file(SMFEnvelope_T *envelope)
 * @brief Get message file
 * @param session SMFSession_T object
 * @returns path to message file
 */
char *smf_session_get_message_file(SMFSession_T *session);

/*!
 * @fn char *smf_envelope_get_id(SMFEnvelope_T *envelope)
 * @brief Get session id
 * @param session SMFSession_T object
 * @returns session id
 */
char *smf_session_get_id(SMFSession_T *session);

#endif  /* _SMF_SESSION_H */
