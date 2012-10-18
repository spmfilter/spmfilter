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
 * @fn *smpf_lookup_sql_connect (SMFSettings_T *settings);
 * @brief connect to sql server
 * @param settings Pointer to SMFSettings_T 
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_sql_connect(SMFSettings_T *settings);

/*!
 * @fn *sql_get_dsn(SMFSettings_T *settings, char *host)
 * @brief Get DSN
 * @param settings Pointer to SMFSettings_T 
 * @param host Host to be used in DSN
 * \returns DSN on success or NULL in case of error
 */
char *sql_get_dsn(SMFSettings_T *settings, char *host);

/*!
 * @fn *smpf_lookup_sql_connect_fallback (SMFSettings_T *settings);
 * @brief fallback_connect to sql server
 * @param settings Pointer to SMFSettings_T 
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_sql_connect_fallback(SMFSettings_T *settings);

/*!
 * @fn *sql_get_rand_host (SMFSettings_T *settings);
 * @brief return random sql backend server
 * @param settings Pointer to SMFSettings_T 
 * \returns hostname/ip of sql server, else NULL 
 */
char *sql_get_rand_host(SMFSettings_T *settings);

/*!
 * @fn *smpf_lookup_sql_disconnect (void);
 * @brief disconnect to sql server
 */
 void smf_lookup_sql_disconnect(void);

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
