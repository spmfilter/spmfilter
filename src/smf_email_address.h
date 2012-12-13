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

/*!
 * @file smf_email_address.h
 * @brief Defines the #SMFEmailAddress_T data type and functions for
 *        E-Mail address handling.
 * @details A #SMFEmailAddress_T consists of three parts: 
 *          - the actual email address, e.g. doe@example.org
 *          - an optional display name, e.g. John Doe
 *          - a possible email addresse type
 * @details To create a new #SMFEmailAddress_T, use smf_email_address_new()
 * @details To destroy a #SMFEmailAddress_T use smf_email_address_free()
 * @details It's possible to parse a string with smf_email_address_parse_string() and create a new
 *          #SMFEmailAddress_T. To get a string from a #SMFEmailAddress_T, use smf_email_address_to_string().
 */

#ifndef _SMF_EMAIL_ADDRESS_H
#define _SMF_EMAIL_ADDRESS_H

#include <cmime.h>

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
 * @typedef SMFEmailAddress_T
 * @brief Represents an E-Mail address.
 */
typedef CMimeAddress_T SMFEmailAddress_T;

/*!
 * @fn SMFEmailAddress_T *smf_email_address_new(void)
 * @brief Creates a new SMFEmailAddress_T object.
 * @return an empty SMFEmailAddress_T object
 */
SMFEmailAddress_T *smf_email_address_new(void);

/*!
 * @fn void smf_email_address_free(SMFEmailAddress_T *ea)
 * @brief Free a SMFEmailAddress_T object.
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
 * @fn SMFEmailAddressType_T smf_email_address_get_type(SMFEmailAddress_T *ea)
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
 * @param ea SMFEmailAddress_T pointer
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

/**
 * @fn int smf_email_address_is_empty(SMFEmailAddress_T *ea)
 * @brief Tests whether the email-part is empty.
 *
 * The function pays attention to the angle-addr format.
 *
 * @param ea The email-instance to check
 * @return When the email-address is emty true is returned
 */
int smf_email_address_is_empty(SMFEmailAddress_T *ea);

/*!
 * @fn SMFEmailAddress_T *smf_email_address_get_clean(SMFEmailAddress_T *ea)
 * @brief Creates a simplified address-instance.
 *
 * The function can be used to extract the email from the angle-addr format.
 *
 * - smf_email_address_get_name() will return NULL
 * - smf_email_address_get_email() will return the simplified email-adress. It
 *   does not have any angle-bracket, this is the plain email.
 *
 * @param ea The SMFEmailAddress_T-instance to clone
 * @return The simplified version of ea
 */
SMFEmailAddress_T *smf_email_address_get_simplified(SMFEmailAddress_T *ea);

#endif  /* _SMF_EMAIL_ADDRESS_H */
