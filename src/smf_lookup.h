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
 * @brief SQL and LDAP lookup functions.
 * @details Spmfilter implements a small, fast, and easy to use database API 
 *         with thread-safe connection pooling. The library can connect transparently 
 *         to multiple database systems, has zero configuration and connections are 
 *         specified via a standard URL scheme.
 *
 * @details Supported are variety of database systems:
 *          - MySQL
 *          - PostgreSQL
 *          - Oracle
 *          - SQLite
 *          - Berkeley DB
 *          - LDAP directories
 * @details Spmfilter cares completely around connection management, load balancing 
 *         and fallback connections. Failed connections will be also reconnected again 
 *         automatically.
 * @details Whether you are using a SQL database or a LDAP directory, all results are 
 *          delivered as a #SMFList_T back. Each element is a #SMFDict_T dictionary, in 
 *          which each key a SQL column or a LDAP attribute represents.
 * @details The only exception here is Berkeley DB, as this does not require a query 
 *          language and is based on key/values.
 * @details In order to use database lookups, you have to set a appropriate backend in 
 *          spmfilter.conf, this can be sql or ldap. If a valid backend is configured, 
 *          spmfilter will automatically establish the connection.
 */

#ifndef _SMF_LOOKUP_H
#define	_SMF_LOOKUP_H

#include "smf_settings.h"
#include "smf_list.h"

/*!
 * @fn char *smf_lookup_db4_query(char *database, char *key)
 * @brief function to get value by key from berkeley hash table
 * @param database path to database
 * @param key key for which the value should be returned
 * @returns newly allocated char pointer value string for given key, NULL if none found
 */
char *smf_lookup_db4_query(char *database, char *key);

/*!
 * @fn int smf_lookup_sql_connect(SMFSettings_T *settings)
 * @brief Open connection to sql server
 * @param settings Pointer to SMFSettings_T 
 * @returns 0 on success or -1 in case of error
 */
int smf_lookup_sql_connect(SMFSettings_T *settings);

/*!
 * @fn void smf_lookup_sql_disconnect(SMFSettings_T *settings);
 * @brief Disconnect from sql server
 * @param settings a SMFSettings_T object
 */
void smf_lookup_sql_disconnect(SMFSettings_T *settings);

/*!
 * @fn SMFList_T *smf_lookup_sql_query(SMFSettings_T *settings, const char *q, ...)
 * @brief Query SQL server with given query string
 * @param settings Pointer to SMFSettings_T
 * @param q format string for sql query for sql query
 * @returns SMFList_T or NULL
 */
SMFList_T *smf_lookup_sql_query(SMFSettings_T *settings, const char *q, ...);

/*!
 * @fn int smf_lookup_ldap_connect(SMFSettings_T *settings)
 * @brief Open a connection to ldap server
 * @param settings Pointer to SMFSettings_T 
 * @returns 0 on success or -1 in case of error
 */
int smf_lookup_ldap_connect(SMFSettings_T *settings);

/*!
 * @fn void smf_lookup_ldap_disconnect(SMFSettings_T *settings)
 * @brief Disconnect from LDAP Server
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
