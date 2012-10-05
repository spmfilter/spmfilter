/* spmfilter - mail filtering framework
 * Copyright (C) 2012 Axel Steiner and SpaceNet AG
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

#ifndef _SMF_SMTP_H
#define _SMF_SMTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "smf_settings.h"
#include "smf_envelope.h"

/*! @struct SMFSmtpStatus_T smf_smtp.h
 * @brief SMTP status informations
 */ 
typedef struct {
    char *text; /**< Text from the server */
    int code; /**< SMTP protocol status code */
} SMFSmtpStatus_T;

/*!
 * @fn SMFSmtpStatus_T *smf_smtp_status_new(void)
 * @brief Creates a new SMFSmtpStatus_T object
 * @returns an empty SMFSmtpStatus_T object
 */
SMFSmtpStatus_T *smf_smtp_status_new(void);

/*!
 * @fn void smf_smtp_status_free(SMFSmtpStatus_T *staus)
 * @brief Free SMFSmtpStatus_T object
 * @param status SMFSmtpStatus_T object
 */
void smf_smtp_status_free(SMFSmtpStatus_T *status);

/*!
 * @fn int smf_smtp_deliver(SMFEnvelope_T *env)
 * @brief Deliver message via smtp
 * @param env a SMFEnvelope_T object
 * @param tls enable/disable TLS for connection
 * @param msg_file alternate message content
 * @returns 0 on success or -1 in case of error
 */
SMFSmtpStatus_T *smf_smtp_deliver(SMFEnvelope_T *env, SMFTlsOption_T tls, char *msg_file); 

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_SMTP_H */