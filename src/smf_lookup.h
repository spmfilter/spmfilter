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
 * @file smf_lookup.h
 * @brief spmfilter lookup functions
 */
#include "smf_settings.h"
#include "smf_list.h"


#ifndef _SMF_LOOKUP_H
#define	_SMF_LOOKUP_H

/*!
 * @fn *smf_lookup_db4_query (char *database, char *key);
 * @brief function to get value by key from berkeley hash table
 * @param key key for which the value should be returned
 * @returns newly allocated char pointer value string for given key, NULL if none found
 */
char *smf_lookup_db4_query(char *database, char *key);

/*!
 * @fn int smf_lookup_db4_update(const char *database, const char *key, const char *value)
 * @brief Updates a key in a berkkey database
 *
 * The database is created if it does not exist. Afterwards the key is created or updated
 * with the given value.
 *
 * @param database The path of the database to update (resp. create)
 * @param key The key to update (resp. create)
 * @param The new value of the key
 * @return On success 0 is returned or -1 in case of an error
 */
int smf_lookup_db4_update(const char *database, const char *key, const char *value);

/*!
 * @fn *smpf_lookup_sql_connect (SMFSettings_T *settings);
 * @brief connect to sql server
 * @param settings Pointer to SMFSettings_T 
 * @returns 0 on success or -1 in case of error
 */
int smf_lookup_sql_connect(SMFSettings_T *settings);

/*!
 * @fn void smf_lookup_sql_disconnect(SMFSettings_T *settings);
 * @brief disconnect from sql server
 * @param settings a SMFSettings_T object
 */
void smf_lookup_sql_disconnect(SMFSettings_T *settings);

/*!
 * @fn *smf_lookup_sql_query (const char *q, ...);
 * @brief Query SQL server with given query string
 * @param settings Pointer to SMFSettings_T
 * @param q format string for sql query for sql query
 * @returns SMFList_T or NULL
 */
SMFList_T *smf_lookup_sql_query(SMFSettings_T *settings, const char *q, ...);

/*!
 * @fn int smf_lookup_ldap_connect(char *settings);
 * @brief connect to ldap server
 * @param settings Pointer to SMFSettings_T 
 * @returns 0 on success or -1 in case of error
 */
int smf_lookup_ldap_connect(SMFSettings_T *settings);

/*!
 * @fn void smf_lookup_ldap_disconnect(char *ldap_uri, SMFSettings_T *settings)
 * @brief disconnect from LDAP Server
 * @param settings Pointer to SMFSettings_T 
 * @returns newly allocated char pointer value string for given key, NULL if none found
 */
void smf_lookup_ldap_disconnect(SMFSettings_T *settings);

/*!
 * @fn SMFList_T *smf_lookup_ldap_query(SMFSettings_T *settings, const char *q, ...)
 * @brief query LDAP Server with given query string
 * @param settings Pointer to SMFSettings_T 
 * @param q format string pointer for ldap query
 * @param ... format string arguments
 * @returns newly allocated SMFList_T
 */
SMFList_T *smf_lookup_ldap_query(SMFSettings_T *settings, const char *q, ...);


#endif	/* _SMF_LOOKUP_H */
