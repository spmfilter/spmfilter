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

#include "spmfilter_config.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_settings.h"
#include "smf_trace.h"

#define THIS_MODULE "lookup"

/** Establish database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_connect(void) {
	SMFSettings_T *settings = smf_settings_get();


	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		if(smf_lookup_sql_connect() != 0)
			return -1;
#endif
	} else if(g_ascii_strcasecmp(settings->backend,"ldap") == 0) {
#ifdef HAVE_LDAP
		if(smf_lookup_ldap_connect() != 0)
			return -1;
#endif
	}

	return 0;
}

/** Destroy database/ldap connection
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_lookup_disconnect(void) {
	SMFSettings_T *settings = smf_settings_get();

	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		smf_lookup_sql_disconnect();
#endif
	} else if (g_ascii_strcasecmp(settings->backend,"ldap") ==  0) {
#ifdef HAVE_LDAP
		smf_lookup_ldap_disconnect();
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
	SMFSettings_T *settings = smf_settings_get();
	
	if(g_ascii_strcasecmp(settings->backend,"sql") == 0) {
#ifdef HAVE_ZDB
		if (settings->sql_user_query != NULL) {
			return smf_lookup_sql_user_exists(addr);
		}
#else
		TRACE(TRACE_ERR,"spmfilter has not been built with sql backend");
		return -1;
#endif
	}

	if (g_ascii_strcasecmp(settings->backend,"ldap") == 0) {
#ifdef HAVE_LDAP
		if (settings->ldap_user_query != NULL) {
			return smf_lookup_ldap_user_exists(addr);
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
 * \return new allocated SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_query(const char *q, ...) {
	va_list ap, cp;
	char *query;
	SMFSettings_T *settings = smf_settings_get();
	
	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);

	if ((g_ascii_strcasecmp(settings->backend,"sql")) == 0) {
#ifdef HAVE_ZDB
		return smf_lookup_sql_query(query);
#else
		TRACE(TRACE_ERR,"spmfilter is not built with sql backend");
		return(NULL);
#endif
	} else if ((g_ascii_strcasecmp(settings->backend,"ldap")) == 0) {
#ifdef HAVE_LDAP
		return smf_lookup_ldap_query(query);
#else
		TRACE(TRACE_ERR,"spmfilter is built with ldap backend");
		return(NULL);
#endif
	} else {
		TRACE(TRACE_ERR,"no valid backend defined");
	}

	return NULL;
}
