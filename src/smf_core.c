/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner, Sebastian Jäkel and SpaceNet AG
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "spmfilter_config.h"
#include "smf_core.h"
#include "smf_settings.h"
#include "smf_trace.h"

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#define THIS_MODULE "utils"
#define GETTIMEOFDAY(t) gettimeofday(t,(struct timezone *) 0)

#define GLIB2_VERSION (GLIB_MAJOR_VERSION * 10000 \
	+ GLIB_MINOR_VERSION * 100 \
	+ GLIB_MICRO_VERSION)

/** extract a substring from given string
 *
 * \param pattern regular expression pattern
 * \param haystack string to search in
 * \param pos position to extract
 *
 * \returns extracted string
 */
char *smf_core_get_substring(const char *pattern, const char *haystack, int pos) {
	if (haystack == NULL)
		return NULL;
#if (GLIB2_VERSION >= 21400)
	GRegex *re = NULL;
	GMatchInfo *match_info = NULL;
	char *value = NULL;

	re = g_regex_new(pattern, G_REGEX_CASELESS, 0, NULL);
	g_regex_match(re, haystack, 0, &match_info);
	if(g_match_info_matches(match_info)) {
		value = g_match_info_fetch(match_info, pos);
	} 
	
	g_match_info_free(match_info);
	g_regex_unref(re);
#else
	pcre *re;
	const char *error;
	int rc, erroffset;
	int ovector[30];
	const char *strptr;
	char *value;

	re = pcre_compile(pattern, PCRE_CASELESS, &error, &erroffset, NULL);
	if(re == NULL) {
		TRACE(TRACE_NOTICE, "pcre_match : failed to compile pattern %s", pattern);
	} else {
		rc = pcre_exec(re, NULL, haystack, strlen(haystack), 0, 0, ovector, 30);
		if(rc > 0) {
			pcre_get_substring(haystack,ovector,rc,pos,&strptr);
			value = g_strdup((char *)strptr);
		} else {
			TRACE(TRACE_ERR, "pcre_match : failed to match pattern %s : code was %d", pattern, rc);
		}
	}
	if (strptr != NULL)
		free((char *) strptr);

#endif	
	return value;
}

/** Generate a new queue file name
 *
 * \buf pointer to unallocated buffer for filename, needs to
 *      free'd by caller if not required anymore
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_core_gen_queue_file(char **tempname) {
	SMFSettings_T *settings = smf_settings_get();
	/* create spooling file */
	*tempname = g_strdup_printf("%s/spmfilter.XXXXXX",settings->queue_dir);
	if(g_mkstemp(*tempname) == -1)
		return -1;
	
	return 0;	
}

/** Generates a unique maildir filename
 *
 * \returns new allocated maildir filename
 */
char *smf_core_get_maildir_filename(void) {
	char *filename;
	char hostname[256];
	struct timeval starttime;
	
	GETTIMEOFDAY(&starttime);
	gethostname(hostname,256);
	
	filename = g_strdup_printf("%lu.V%lu.%s",
		(unsigned long) starttime.tv_sec,
		(unsigned long) starttime.tv_usec,
		hostname);
	
	return filename;
}
