#include <glib.h>
#include <string.h>

#include "spmfilter.h"
#include "lookup.h"

#define THIS_MODULE "lookup"

/** expands placeholders in a user querystring
 *
 * \param format format string to use as input
 * \param addr email address to use for replacements
 * \buf pointer to unallocated buffer for expanded format string, needs to
 *      free'd by caller if not required anymore
 *
 * \returns the number of replacements made or -1 in case of error
 */
int expand_query(char *format, char *addr, char **buf) {
	int rep_made = 0;
	int pos = 0;
	int iter_size;
	char *it = format;
	char *iter;
	gchar **parts = g_strsplit(addr, "@", 2);

	/* allocate space for buffer
	 * TODO: put buffer size declaration somewhere else
	 */
	*buf = (char *)calloc(512,sizeof(char));
	if(*buf == NULL) {
		return(-1);
	}

	while(*it != '\0') {
		if(*it == '%') {
			*it++;
			switch(*it) {
				case 's':
					iter = addr;
					break;
				case 'u':
					iter = parts[0];
					break;
				case 'd':
					iter = parts[1];
					break;
				default:
					return(-2);
					break; /* never reached */
			}

			/* now copy the replacement text */
			iter_size = strlen(iter);
			memcpy((*buf + pos), iter, iter_size);
			pos += iter_size;

			*it++; /* jump over current */
			rep_made++;
		} else {
			(*buf)[pos++] = *it++;
		}
	}

	g_strfreev(parts);
	return(rep_made);
}

/** Establish database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int lookup_connect(void) {
	Settings_T *settings = get_settings();

#ifdef HAVE_ZDB
	/* try to connect to database */
	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
		if(sql_connect() != 0)
			return -1;
	}
#endif

#ifdef HAVE_LDAP
	if(g_ascii_strcasecmp(settings->backend,"ldap") == 0) {
		if(ldap_connect() != 0)
			return -1;
	}
#endif

	return 0;
}

/** Destroy database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int lookup_disconnect(void) {
	Settings_T *settings = get_settings();

#ifdef HAVE_ZDB
	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
		sql_disconnect();
	}
#endif

#ifdef HAVE_LDAP
	if (g_ascii_strcasecmp(settings->backend,"ldap") ==  0) {
		ldap_disconnect();
	}
#endif

	return 0;
}

/** Check if given user is local
 *
 * \param addr email address
 *
 * \returns 0 if user is not local, 1 if
 *          user is local
 */
int lookup_user(char *addr) {
	Settings_T *settings = get_settings();
#ifdef HAVE_ZDB
	if((g_ascii_strcasecmp(settings->backend,"sql") == 0)
			&& (settings->sql_user_query != NULL)) {
		return sql_user_exists(addr);
	}
#endif

#ifdef HAVE_LDAP
	if((g_ascii_strcasecmp(settings->backend,"ldap") == 0)
			&& (settings->ldap_user_query != NULL)) {
		return ldap_user_exists(addr);
	}

#endif
	return 0;
}
