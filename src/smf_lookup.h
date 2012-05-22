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


/*!
 * @file smf_lookup.h
 * @brief spmfilter lookup functions
 */

#include "smf_settings.h"

#ifndef _SMF_LOOKUP_H
#define	_SMF_LOOKUP_H

typedef struct {
	void *data;
} SMFLookupElement_T;

typedef struct {
	SMFLookupElement_T *elem;
	unsigned int len;
} SMFLookupResult_T;


/** Since LDAP attributes can have more than
 *  one value, we need to define a new datatype,
 *  in order not to link every module against
 *  ldap, which needs LDAP connectivity.
 *  Every LDAP lookup returns SMFLookupElement_T
 *  with LdapValue_T pointer as data.
 */
typedef struct {
	int len;
	char **data;
} SMFLdapValue_T;

/** Frees all of the memory used by SMFLookupResult_T
 *
 * \param *l pointer to SMFLookupResult_T
 */
void smf_lookup_result_free(SMFLookupResult_T *l);

/** Returns SMFLookupElement_T at the given index of SMFLookupResult_T
 *
 * \param l SMFLookupResult_T list
 * \param i the index of the pointer to return
 *
 * \returns SMFLookupElement_T at the given index
 */
SMFLookupElement_T *smf_lookup_result_index(SMFLookupResult_T *l, int i);

/** Looks up a key in SMFLookupElement_T
 *
 * \param *e pointer to SMFLookupElement_T
 * \param *key the key to look up
 *
 * \returns the associated value, or NULL if the key is not found
 */
void *smf_lookup_element_get(SMFLookupElement_T *e, char *key);

/** Query lookup backend.
 *
 * \param q a standard printf() format string
 * \param args the list of parameters to insert into the format string
 *
 * \return new allocated SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_query(const char *q, ...);

/** Establish database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_connect(void);

/** Destroy database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_disconnect(void);

/** Query Berkeley DB for given key
 *
 * \param database path to database file
 * \param key key to lookup for
 *
 * \returns the values associated with key,  or NULL if the key is not found
 */

/*!
 * @fn *smf_lookup_db4_query (char *database, char *key);
 * @brief function to get value by key from berkeley hash table
 * @param key key for which the value should be returned
 * @returns newly allocated char pointer value string for given key, NULL if none found
 */
char *smf_lookup_db4_query(char *database, char *key);

/** Connect to sql server
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_sql_connect(void);

/** Disconnect from sql server */
void smf_lookup_sql_disconnect(void);

/** Query SQL server with given query string
 *
 * \param q format string for sql query
 * \param ... format string arguments
 *
 * \returns SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_sql_query(const char *q, ...);

/** Connect to ldap server
 *
 * \returns 0 on success or -1 in case of error
 */

/*!
 * @fn int smf_lookup_ldap_connect(char *ldap_uri);
 * @brief connect to ldap server
 * @param ldap_uri LDAP-URI
 * @param ldap_uri Pointer to SMFSettings_T 
 * @returns 0 on success or -1 in case of error
 */
int smf_lookup_ldap_connect(char *ldap_uri, SMFSettings_T *settings);

/** Disconnect from LDAP server */
void smf_lookup_ldap_disconnect(void);

/** Query LDAP server with given query string
 *
 * \param q format string for ldap query
 * \param ... format string arguments
 *
 * \returns SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_ldap_query(const char *q, ...);

#endif	/* _SMF_LOOKUP_H */
