#include <string.h>
#include <syslog.h>
#include <glib.h>

#include "spmfilter.h"

char *get_substring(const char *pattern, const char *haystack, int pos) {
	GRegex *re;
	GMatchInfo *match_info;
	char *value = NULL;

	re = g_regex_new(pattern, G_REGEX_CASELESS, 0, NULL);
	g_regex_match(re, haystack, 0, &match_info);
	if(g_match_info_matches(match_info)) {
		value = g_match_info_fetch(match_info, pos);
	} else {
		syslog(LOG_DEBUG,"NO match");
	}
	g_match_info_free(match_info);
	g_regex_unref(re);
	return value;
}

void header_check (gpointer k, gpointer v, gpointer user_data) {
	HL *header_lookup = user_data;
	gchar *key = k;
	GList *keys, *l;
	HEADER *header;

	header = g_hash_table_lookup(header_lookup->settings->header_checks,k);
	header->value = g_mime_message_get_header(header_lookup->message,header->name);
	if (header->value != NULL) {
		if (header_lookup->settings->debug) {
			syslog(LOG_DEBUG,"found header: %s",header->name);
			syslog(LOG_DEBUG,"mconn->header_results: added %s:%s", k,header->value);
		}
	}
}
