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

#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "spmfilter.h"
#include "smf_lookup.h"

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
	Settings_T *settings = smf_settings_get();


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
	Settings_T *settings = smf_settings_get();

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
int smf_lookup_check_user(char *addr) {
	Settings_T *settings = smf_settings_get();

	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		if (settings->sql_user_query != NULL) {
			return sql_user_exists(addr);
		}
#else
		TRACE(TRACE_ERR,"spmfilter has not been built with sql backend");
		return -1;
#endif
	}

	if (g_ascii_strcasecmp(settings->backend,"ldap") == 0) {
#ifdef HAVE_LDAP
		if (settings->ldap_user_query != NULL) {
			return ldap_user_exists(addr);
		}
#else
		TRACE(TRACE_ERR,"spmfilter has not been built with ldap backend");
		return -1;
#endif
	}

	TRACE(TRACE_ERR,"no valid backend defined");
	return -1;
}

/** Query lookup backend.
 *
 * \param q a standard printf() format string
 * \param args the list of parameters to insert into the format string
 *
 * \return new allocated LookupResult_T
 */
LookupResult_T *smf_lookup_query(const char *q, ...) {
	va_list ap, cp;
	char *query;
	Settings_T *settings = smf_settings_get();
	
	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);

	if ((g_ascii_strcasecmp(settings->backend,"sql")) == 0) {
#ifdef HAVE_ZDB
		return sql_query(query);
#else
		TRACE(TRACE_ERR,"spmfilter is not built with sql backend");
		return(NULL);
#endif
	} else if ((g_ascii_strcasecmp(settings->backend,"ldap")) == 0) {
#ifdef HAVE_LDAP
		return ldap_query(query);
#else
		TRACE(TRACE_ERR,"spmfilter is built with ldap backend");
		return(NULL);
#endif
	} else {
		TRACE(TRACE_ERR,"no valid backend defined");
	}

	return NULL;
}
