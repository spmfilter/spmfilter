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
int sql_connect(void);
void sql_disconnect(void);
#endif
#endif	/* _LOOKUP_H */

