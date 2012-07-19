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

#ifndef _SMF_EMAIL_ADDRESS_H
#define _SMF_EMAIL_ADDRESS_H

#include <glib.h>
#include <cmime.h>

#include "spmfilter_config.h"
#include "smf_lookup.h"

/*!
 * @enum SMFEmailAddressType_T 
 * @brief Possible types of email addresses
 */
typedef enum _SMFEmailAddressType {
        SMF_EMAIL_ADDRESS_TYPE_TO, /**< message recipient */
        SMF_EMAIL_ADDRESS_TYPE_CC, /**< message cc recipient */
        SMF_EMAIL_ADDRESS_TYPE_BCC, /**< nessage bcc recipient */
        SMF_EMAIL_ADDRESS_TYPE_FROM, /**< message sender */
} SMFEmailAddressType_T;

/*!
 * @struct SMFEmailAddress_T smf_email_address.h
 * @brief Represents an E-Mail address
 */
typedef CMimeAddress_T SMFEmailAddress_T;

/*!
 * @fn SMFEmailAddress_T *smf_email_address_new(void)
 * @brief Creates a new SMFEmailAddress_T object
 * @returns an empty SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_new(void);

/*!
 * @fn void smf_email_address_free(SMFEmailAddress_T *ea)
 * @brief Free a SMFEmailAddress_T object
 * @param ea a SMFEmailAddress_T object
 */
void smf_email_address_free(SMFEmailAddress_T *ea);

/*!
 * @fn SMFEmailAddress_T *smf_email_address_parse_string(const char *addr)
 * @brief Parse given string and create SMFEmailAddress_T object
 * @param addr E-Mail address
 * @returns a newly allocated SMFEmailAddress_T object, or NULL on failure
 */
SMFEmailAddress_T *smf_email_address_parse_string(const char *addr);

/*!
 * @fn char *smf_email_address_to_string(SMFEmailAddress_T *ea)
 * @brief Allocates a string containing the contents of the SMFEmailAddress_T object.
 * @param ea SMFEmailAddress_T object
 * @returns the SMFEmailAddress_T object as an newly allocated string in rfc822 format.
 */
char *smf_email_address_to_string(SMFEmailAddress_T *ea);

/*!
 * @fn void smf_email_address_set_type(SMFEmailAddress_T *ea, SMFEmailAddressType_T t)
 * @brief Set the address type of SMFEmailAddress_T object
 * @param ea SMFEmailAddress_T object
 * @param t SMFEmailAddressType_T type
 */
void smf_email_address_set_type(SMFEmailAddress_T *ea, SMFEmailAddressType_T t);

/*!
 * @fn SMFEmailAddressType_T smf_email_address_get_type(SMFEMailAddress_T *ea)
 * @brief Get the address type of SMFEmailAddress_T object
 * @param ea SMFEmailAddress_T object
 * @returns a SMFEmailAddressType_T object
 */
SMFEmailAddressType_T smf_email_address_get_type(SMFEmailAddress_T *ea);

/*! 
 * @fn void smf_email_address_set_name(SMFEmailAddress_T *ea, const char *name)
 * @brief Set the display name of SMFEmailAddress_T object
 * @param ea SMFEmailAddress_T pointer
 * @param name the display name for the address
 */
void smf_email_address_set_name(SMFEmailAddress_T *ea, const char *name);

/*!
 * @fn char *smf_email_address_get_name(SMFEmailAddress_T *ea)
 * @brief Get the display name of a SMFEmailAddress_T object
 * @param ea a SMFEmailAddress_T object
 * @returns the display name of a SMFEmailAddress_T object
 */
char *smf_email_address_get_name(SMFEmailAddress_T *ea);

/*! 
 * @fn void smf_email_address_set_email(SMFEmailAddress_T *ea, const char *email)
 * @brief Set the email address of SMFEmailAddress_T object
 * @param ca SMFEmailAddress_T pointer
 * @param email email address
 */
void smf_email_address_set_email(SMFEmailAddress_T *ea, const char *email);

/*!
 * @fn char *smf_email_address_get_email(SMFEmailAddress_T *ea)
 * @brief Get the email address of a SMFEmailAddress_T object
 * @param ea a SMFEmailAddress_T object
 * @returns the email address of a SMFEmailAddress_T object
 */
char *smf_email_address_get_email(SMFEmailAddress_T *ea);

#endif  /* _SMF_EMAIL_ADDRESS_H */
