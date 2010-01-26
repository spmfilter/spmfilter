#ifndef _LOOKUP_H
#define	_LOOKUP_H

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

#ifdef HAVE_ZDB
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
#endif

#ifdef HAVE_LDAP
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
#endif

#endif	/* _LOOKUP_H */

