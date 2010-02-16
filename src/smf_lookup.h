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

#ifndef _SMF_LOOKUP_H
#define	_SMF_LOOKUP_H


typedef struct {
	void *data;
} LookupElement_T;

typedef struct {
	LookupElement_T *elem;
	unsigned int len;
} LookupResult_T;

/** Since LDAP attributes can have more than
 *  one value, we need to define a new datatype,
 *  in order not to link every module against
 *  ldap, which needs LDAP connectivity.
 *  Every LDAP lookup returns LookupElement_T
 *  with LdapValue_T pointer as data.
 */
typedef struct {
	int len;
	char **data;
} LdapValue_T;

/** Frees all of the memory used by LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 */
void smf_lookup_result_free(LookupResult_T *l);

/** Returns LookupElement_T at the given index of LookupResult_T
 *
 * \param l LookupResult_T list
 * \param i the index of the pointer to return
 *
 * \returns LookupElement_T at the given index
 */
LookupElement_T *smf_lookup_result_index(LookupResult_T *l, int i);

/** Looks up a key in LookupElement_T
 *
 * \param *e pointer to LookupElement_T
 * \param *key the key to look up
 *
 * \returns the associated value, or NULL if the key is not found
 */
void *smf_lookup_element_get(LookupElement_T *e, char *key);

/** Check if given user is local
 *
 * \param addr email address
 *
 * \returns 0 if user is not local, 1 if
 *          user is local
 */
int smf_lookup_check_user(char *addr);

/** Query lookup backend.
 *
 * \param q a standard printf() format string
 * \param args the list of parameters to insert into the format string
 *
 * \return new allocated LookupResult_T
 */
LookupResult_T *smf_lookup_query(const char *q, ...);

/** Query Berkeley DB for given key
 *
 * \param database path to database file
 * \param key key to lookup for
 *
 * \returns the values associated with key,  or NULL if the key is not found
 */
char *smf_db4_query(char *database, char *key);


/** expands placeholders in a user querystring
 *
 * \param format format string to use as input
 * \param addr email address to use for replacements
 * \buf pointer to unallocated buffer for expanded format string, needs to
 *      free'd by caller if not required anymore
 *
 * \returns the number of replacements made or -1 in case of error
 */
int expand_query(char *format, char *addr, char **buf);

/** Establish database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int lookup_connect(void);

/** Destroy database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int lookup_disconnect(void);

/** Allocates memory for LookupResult_T
 *
 * \returns newly allocated LookupResult_T
 */
LookupResult_T *lookup_result_new(void);

/** Adds a new element on to the end of the list.
 *  The return value is the new start of the list,
 *  which may have changed, so make sure you store the new value.
 *
 * \param *l pointer to LookupResult_T
 * \param *elem_data pointer to LookupElement_T
 */
void lookup_result_add(LookupResult_T *l, LookupElement_T *elem_data);

/** Creates a new element for LookupResult_T
 *
 * \returns pointer to new allocated LookupRow_T
 */
LookupElement_T *lookup_element_new(void);

/** Inserts a new key and value into LookupElement_T
 *  If the key already exists in LookupElement_T its current value is
 *  replaced with the new value.
 *
 * \param *e pointer to LookupElement_T
 * \param *key a key to insert
 * \param *value the value to associate with the key
 */
void lookup_element_add(LookupElement_T *e, char *key, void *value);

/** Connect to sql server
 *
 * \returns 0 on success or -1 in case of error
 */
int sql_connect(void);

void sql_disconnect(void);

/** Check if given user exists in database
 *
 * \param addr email adress of user
 *
 * \return 1 if the user exists, otherwise 0
 */
int sql_user_exists(char *addr);

LookupResult_T *sql_query(const char *q, ...);

/** Connect to ldap server
 *
 * \returns 0 on success or -1 in case of error
 */
int ldap_connect(void);

void ldap_disconnect(void);

/** Check if given user exists in ldap directory
 *
 * \param addr email adress of user
 *
 * \return 1 if the user exists, otherwise 0
 */
int ldap_user_exists(char *addr);

LookupResult_T *ldap_query(const char *q, ...);

#endif	/* _SMF_LOOKUP_H */

