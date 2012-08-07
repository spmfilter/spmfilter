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

#ifndef _SMF_ENVELOPE_H
#define _SMF_ENVELOPE_H

#include "smf_list.h"
#include "smf_email_address.h"
#include "smf_message.h"

/*!
 * @struct SMFEnvelope_T smf_message.h
 * @brief Message envelope object 
 */
typedef struct {
    SMFList_T *recipients; /**< envelope recipients */
    SMFEmailAddress_T *sender; /**< envelope sender */
    char *message_file; /**< path to message */
    char *auth_user; /**< SMTP auth user, if needed */
    char *auth_pass; /**< SMTP auth password, if needed */
    char *nexthop; /**< destination smtp server */
    SMFMessage_T *message; /**< related message object */
} SMFEnvelope_T;

/*!
 * @fn SMFEnvelope_T *smf_envelope_new(void)
 * @brief Creates a new SMFEnvelope_T object
 * @returns an empty SMFEnvelope_T object
 */
SMFEnvelope_T *smf_envelope_new(void);

/*!
 * @fn void smf_envelope_free(SMFEnvelope_T *envelope)
 * @brief Free SMFEnvelope_T object
 * @param envelope SMFEnvelope_T object
 */
void smf_envelope_free(SMFEnvelope_T *envelope);

/*!
 * @fn void smf_envelope_set_sender(SMFEnvelope_T *envelope, char *sender)
 * @brief Set sender to envelope
 * @param envelope SMFEnvelope_T object
 * @param sender envelope sender address
 */
void smf_envelope_set_sender(SMFEnvelope_T *envelope, char *sender);

/*!
 * @fn char *smf_envelope_get_sender_string(SMFEnvelope_T *envelope)
 * @brief Get envelope sender as string
 * @param envelope SMFEnvelope_T object
 * @returns envelope sender
 */
char *smf_envelope_get_sender_string(SMFEnvelope_T *envelope);

/*!
 * @fn SMFEmailAddress_T *smf_envelope_get_sender(SMFEnvelope_T *envelope)
 * @brief Get envelope sender 
 * @param envelope SMFEnvelope_T object
 * @returns SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_envelope_get_sender(SMFEnvelope_T *envelope);

/*!
 * @fn int smf_envelope_add_rcpt(SMFEnvelope_T *envelope, char *rcpt)
 * @brief Add new recipient to envelope
 * @param envelope SMFEnvelope_T object
 * @param rcpt rcpt address
 * @returns 0 on success or -1 in case of error
 */
int smf_envelope_add_rcpt(SMFEnvelope_T *envelope, char *rcpt);

/*!
 * @fn typedef void (*SMFRcptForeachFunc) (SMFEmailAddress_T *ea, void *user_data)
 * @brief The function signature for a callback to smf_envelope_foreach_rcpt()
 * @param ea a SMFAddress_T object
 * @param user_data User-supplied callback data.
 */
typedef void (*SMFRcptForeachFunc) (SMFEmailAddress_T *ea, void *user_data);

/*!
 * @fn void smf_envelope_foreach_rcpt(SMFEnvelope_T *envelope, SMFRcptForeachFunc callback, void  *user_data)
 * @brief Recursively calls callback on each envelope recipient.
 * @param envelope SMFEnvelope_T object
 * @param callback function to call on each recipient
 * @param user-supplied callback data
 */
void smf_envelope_foreach_rcpt(SMFEnvelope_T *envelope, SMFRcptForeachFunc callback, void  *user_data);

/*!
 * @fn void smf_envelope_set_message_file(SMFEnvelope_T *envelope, char *fp)
 * @brief Set path for message file
 * @param envelope SMFEnvelope_T object
 * @param fp message file path
 */
void smf_envelope_set_message_file(SMFEnvelope_T *envelope, char *fp);

/*!
 * @fn char *smf_envelope_get_message_file(SMFEnvelope_T *envelope)
 * @brief Get message file
 * @param envelope SMFEnvelope_T object
 * @returns path to message file
 */
char *smf_envelope_get_message_file(SMFEnvelope_T *envelope);

/*!
 * @fn void smf_envelope_set_auth_user(SMFEnvelope_T *envelope,char *auth_user)
 * @brief Set auth user
 * @param envelope SMFEnvelope_T object
 * @param auth_user Auth username
 */
void smf_envelope_set_auth_user(SMFEnvelope_T *envelope,char *auth_user);


/*!
 * @fn char *smf_envelope_get_auth_user(SMFEnvelope_T *envelope)
 * @brief Get auth user
 * @param envelope SMFEnvelope_T object
 * @returns auth username
 */
char *smf_envelope_get_auth_user(SMFEnvelope_T *envelope);

/*!
 * @fn void smf_envelope_set_auth_pass(SMFEnvelope_T *envelope, char *auth_pass)
 * @brief Set auth password
 * @param envelope SMFEnvelope_T object
 * @param auth_pass Auth password
 */
void smf_envelope_set_auth_pass(SMFEnvelope_T *envelope, char *auth_pass);


/*!
 * @fn char *smf_envelope_get_auth_pass(SMFEnvelope_T *envelope)
 * @brief Get auth pass
 * @param envelope SMFEnvelope_T object
 * @returns auth password
 */
char *smf_envelope_get_auth_pass(SMFEnvelope_T *envelope);

/*!
 * @fn void smf_envelope_set_nexthop(SMFEnvelope_T *envelope, char *nexthop)
 * @brief Set nexthop
 * @param envelope SMFEnvelope_T object
 * @param nexthop nexthop
 */
void smf_envelope_set_nexthop(SMFEnvelope_T *envelope, char *nexthop);

/*!
 * @fn char *smf_envelope_get_nexthop(SMFEnvelope_T *nexthop)
 * @brief Get nexthop
 * @param envelope SMFEnvelope_T object
 * @returns nexthop
 */
char *smf_envelope_get_nexthop(SMFEnvelope_T *nexthop);

#endif  /* _SMF_ENVELOPE_H */
