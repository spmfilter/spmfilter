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

#ifndef _SMF_LOOKUP_PRIVATE_H
#define	_SMF_LOOKUP_PRIVATE_H

#include "smf_session.h"

/** Allocates memory for SMFLookupResult_T
 *
 * \returns newly allocated SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_result_new(void);

/** Adds a new element on to the end of the list.
 *  The return value is the new start of the list,
 *  which may have changed, so make sure you store the new value.
 *
 * \param *l pointer to SMFLookupResult_T
 * \param *elem_data pointer to SMFLookupElement_T
 */
void smf_lookup_result_add(SMFLookupResult_T *l, SMFLookupElement_T *elem_data);

/** Creates a new element for SMFLookupResult_T
 *
 * \returns pointer to new allocated SMFLookupRow_T
 */
SMFLookupElement_T *smf_lookup_element_new(void);

/** Inserts a new key and value into SMFLookupElement_T
 *  If the key already exists in SMFLookupElement_T its current value is
 *  replaced with the new value.
 *
 * \param *e pointer to SMFLookupElement_T
 * \param *key a key to insert
 * \param *value the value to associate with the key
 */
void smf_lookup_element_add(SMFLookupElement_T *e, char *key, void *value);

/** Check if given user is local
 *
 * \param user a SMFEmailAddress_T object
 */
void smf_lookup_check_user(SMFEmailAddress_T *user);

/** Check if given user exists in ldap directory
 *
 * \param user a SMFEmailAddress_T object
 */
void smf_lookup_ldap_check_user(SMFEmailAddress_T *user);

/** Check if given user exists in database
 *
 * \param user a SMFEmailAddress_T object
 */
void smf_lookup_sql_check_user(SMFEmailAddress_T *user);

/*!
 * @fn int ldap_get_scope(char *ldap_scope)
 * @brief function to get ldap_scope 
 * @param ldap_scope 
 * @returns ldap_scope
 */
int ldap_get_scope(char *ldap_scope);

/*!
 * @fn char *ldap_get_uri(char *ldap_host, char *ldap_port);
 * @brief function to get ldap_uri
 * @param ldap_host LDAP-Host
 * @param ldap_port LDAP-Port
 * @returns ldap_uri
 */
char *ldap_get_uri(char *ldap_host, char *ldap_port);


#endif	/* _SMF_LOOKUP_PRIVATE_H */

