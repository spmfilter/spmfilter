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
#include "smf_core.h"

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#define THIS_MODULE "lookup"

int smf_lookup_connect(void) {
	int i;
	SMFSettings_T *settings = smf_settings_get();

	for (i=0; settings->backend[i] != NULL; i++) {
#ifdef HAVE_ZDB
		if(g_ascii_strcasecmp(settings->backend[i],"sql") == 0) {
			if(smf_lookup_sql_connect() != 0)
				return -1;
		}
#endif

#ifdef HAVE_LDAP
		if(g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) {
			if(smf_lookup_ldap_connect() != 0)
				return -1;
		}
#endif
	}
	
	return 0;
}

/** Destroy database/ldap connection */
int smf_lookup_disconnect(void) {
	int i;
	SMFSettings_T *settings = smf_settings_get();

	for (i=0; settings->backend[i] != NULL; i++) {
#ifdef HAVE_ZDB
		if(g_ascii_strcasecmp(settings->backend[i],"sql") == 0)
			smf_lookup_sql_disconnect();
#endif

#ifdef HAVE_LDAP
		if (g_ascii_strcasecmp(settings->backend[i],"ldap") ==  0)
			smf_lookup_ldap_disconnect();
#endif
	}

	return 0;
}

/** Check if given user is local */
void smf_lookup_check_user(SMFEmailAddress_T *user) {
	int i;
	SMFSettings_T *settings = smf_settings_get();

	for (i=0; settings->backend[i] != NULL; i++) {
#ifdef HAVE_ZDB
		if(g_ascii_strcasecmp(settings->backend[i],"sql") == 0) {
			if (settings->sql_user_query != NULL) {
				smf_lookup_sql_check_user(user);
			}
		}
#endif

#ifdef HAVE_LDAP
		if (g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) {
			if (settings->ldap_user_query != NULL) {
				// aufruf noch falsch
				smf_lookup_ldap_check_user(user);
			}
		}
#endif
	}

	if (user->lr != NULL) {
		if (user->lr->len != 0)
			user->is_local = 1;
	} else
		user->is_local = 0;
}

/** Query lookup backend. */
SMFLookupResult_T *smf_lookup_query(const char *q, ...) {
	va_list ap, cp;
	char *query = NULL;
	gboolean is_sql = FALSE;
	int i;
	SMFLookupResult_T *result = NULL;
	SMFSettings_T *settings = smf_settings_get();

#if (GLIB2_VERSION >= 21400)
	GRegex *re = NULL;
	GMatchInfo *match_info = NULL;
#else
	pcre *re;
	int rc;
	int ovector[30];
#endif
	
	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
	
#if (GLIB2_VERSION >= 21400)
	re = g_regex_new("/(^SELECT|^UPDATE|^DELETE|^INSERT|^DROP|^CREATE|^ALTER)/", G_REGEX_CASELESS, 0, NULL);
	g_regex_match(re, query, 0, &match_info);
	if(g_match_info_matches(match_info)) {
		is_sql = TRUE;
	}
	g_match_info_free(match_info);
	g_regex_unref(re);
#else
	re = pcre_compile("/(^SELECT|^UPDATE|^DELETE|^INSERT)/", PCRE_CASELESS, NULL, NULL, NULL);
	if(re != NULL) {
		rc = pcre_exec(re, NULL, query, strlen(query), 0, 0, ovector, 30);
		if(rc > 0) {
			is_sql = TRUE;
		}
	}
#endif

	
	for (i=0; settings->backend[i] != NULL; i++) {
#ifdef HAVE_ZDB
		if ((g_ascii_strcasecmp(settings->backend[i],"sql") == 0) && (is_sql == TRUE)) {
			result = smf_lookup_sql_query(query);
			break;
		}
#endif

#ifdef HAVE_LDAP
		if ((g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) && (is_sql == FALSE)) {

			// TODO - fix 
			result = smf_lookup_ldap_query(query);
			break;
		}
#endif
	}
	g_free(query);
	return result;
}
