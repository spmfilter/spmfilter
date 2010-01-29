#include <glib.h>
#include <string.h>
#include <stdlib.h>

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


	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		if(sql_connect() != 0)
			return -1;
#endif
	} else if(g_ascii_strcasecmp(settings->backend,"ldap") == 0) {
#ifdef HAVE_LDAP
		if(ldap_connect() != 0)
			return -1;
#endif
	}

	return 0;
}

/** Destroy database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int lookup_disconnect(void) {
	Settings_T *settings = get_settings();

	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		sql_disconnect();
#endif
	} else if (g_ascii_strcasecmp(settings->backend,"ldap") ==  0) {
#ifdef HAVE_LDAP
		ldap_disconnect();
#endif
	}

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

	if((g_ascii_strcasecmp(settings->backend,"sql") == 0)
			&& (settings->sql_user_query != NULL)) {
#ifdef HAVE_ZDB
		return sql_user_exists(addr);
#else
		TRACE(TRACE_ERR,"spmfilter is not built with sql backend");
		return -1;
#endif
	} else if ((g_ascii_strcasecmp(settings->backend,"ldap") == 0)
			&& (settings->ldap_user_query != NULL)) {
#ifdef HAVE_LDAP
		return ldap_user_exists(addr);
#else
		TRACE(TRACE_ERR,"spmfilter is not built with ldap backend");
		return -1;
#endif
	} else {
		TRACE(TRACE_ERR,"no valid backend defined");
		return -1;
	}

}

void lookup_query(const char *q, ...) {
	Settings_T *settings = get_settings();

	if ((g_ascii_strcasecmp(settings->backend,"sql")) == 0) {
#ifdef HAVE_ZDB
		// TODO: implement sql_query
#else
		TRACE(TRACE_ERR,"spmfilter is not built with sql backend");
		return;
#endif
	} else if ((g_ascii_strcasecmp(settings->backend,"ldap")) == 0) {
#ifdef HAVE_LDAP
		// TODO: implement ldap_query
#else
		TRACE(TRACE_ERR,"spmfilter is built with ldap backend");
		return;
#endif
	} else {
		TRACE(TRACE_ERR,"no valid backend defined");
		return;
	}
}
