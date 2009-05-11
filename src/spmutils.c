#include <string.h>
#include <pcre.h>
#include <syslog.h>
#include <glib.h>

#include "spmfilter.h"

char *get_substring(const char *pattern, const char *line, int pos) {
	pcre *re;
	const char *error;
	int rc, erroffset;
	int ovector[30];
	const char **strptr;
	char *value = NULL;
	char *haystack;
	
	haystack = g_strdup(line);
	re = pcre_compile(pattern, PCRE_CASELESS, &error, &erroffset, NULL);
	if(re == NULL) {
		syslog(LOG_NOTICE, "pcre_match : failed to compile pattern %s", pattern);
	} else {
		rc = pcre_exec(re, NULL, haystack, strlen(haystack), 0, 0, ovector, 30);
		if(rc > 0) {
			pcre_get_substring(haystack,ovector,rc,pos,strptr);
			value = g_strdup(strptr[0]);
		} else {
			syslog(LOG_ERR, "pcre_match : failed to match pattern %s : code was %d", pattern, rc);
		}
	}
	
	g_free(haystack);
	return value;
}

