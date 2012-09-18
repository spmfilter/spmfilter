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


/** Check if given user is local
 *
 * \param user a SMFEmailAddress_T object
 */
void smf_lookup_check_user(SMFEmailAddress_T *user);

/*!
 * @fn void smf_lookup_ldap_check_user(SMFSettings_T *settings, SMFEmailAddress_T *user)
 * @brief check if given user exists in ldap directory
 * @param ldap_uri Pointer to ldap_uri
 * @param SMFEmailAddress_T Pointer
 * @param SMFSettings_T Pointer
 */
void smf_lookup_ldap_check_user(char *ldap_uri, SMFEmailAddress_T *user,SMFSettings_T *settings);

/*!
 * @fn void smf_lookup_sql_check_user(SMFSettings_T *settings, SMFEmailAddress_T *user)
 * @brief check if given user exists in ldap directory
 * @param ldap_uri Pointer to ldap_uri
 * @param SMFSettings_T Pointer
 * @param SMFEmailAddress_T Pointer
 */
void smf_lookup_sql_check_user(SMFSettings_T *settings, SMFEmailAddress_T *user);

/*!
 * @fn int ldap_get_scope(SMFSettings_T *settings)
 * @brief function to get ldap_scope 
 * @param SMFSettings_T Pointer
 * @returns ldap_scope
 */
int ldap_get_scope(SMFSettings_T *settings);

/*!
 * @fn char *ldap_get_uri(char *ldap_host, char *ldap_port);
 * @brief function to get ldap_uri
 * @param ldap_host LDAP-Host
 * @param ldap_port LDAP-Port
 * @returns ldap_uri
 */
char *ldap_get_uri(char *ldap_host, int ldap_port);

/*!
 * @fn char *ldap_get_rand_host(SMFSettings_T *settings);
 * @brief function to get random ldap host
 * @param SMFSettings_T Pointer
 * @returns random ldap host
 */
char *ldap_get_rand_host(SMFSettings_T *settings);

/*!
 * @fn int smf_ldap_bind(SMFSettings_T *settings);
 * @brief function to get random ldap host
 * @param SMFSettings_T Pointer
 * @returns random ldap host
 */
int smf_ldap_bind(SMFSettings_T *settings);

/*!
 * @fn int smf_ldap_bind(char *uri, SMFSettings_T *settings);
 * @brief Try to get a failover connection to other server
 * @param SMFSettings_T Pointer
 * @returns 0 on success or -1 in case of error
 */
int ldap_failover_connect(SMFSettings_T *settings);



#endif	/* _SMF_LOOKUP_PRIVATE_H */

