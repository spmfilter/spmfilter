#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_PCRE
#include <pcre.h>
#endif

#include "spmfilter.h"

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
char *get_substring(const char *pattern, const char *haystack, int pos) {
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
	int rc, erroffset, i;
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
	if (error != NULL)
		free(error);
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
int gen_queue_file(char **tempname) {
	Settings_T *settings = get_settings();
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
char *get_maildir_filename(void) {
	char *filename;
	char hostname[256];
	struct timeval starttime;
	
	GETTIMEOFDAY(&starttime);
	gethostname(hostname,256);
	
	filename = g_strdup_printf("%lu.V%lu.%s",
		(unsigned long) starttime.tv_sec,
		(unsigned long) starttime.tv_usec,
		hostname);
	
	g_free(hostname);
	return filename;
}

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
	*buf = (char *)g_malloc(sizeof(char));
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
