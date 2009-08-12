#include <string.h>
#include <syslog.h>
#include <glib.h>
#include <stdio.h>
#include <fcntl.h>
#if (GLIB2_VERSION < 21400)
#include <pcre.h>
#endif
#include <gmime/gmime.h>

#include "spmfilter.h"

char *get_substring(const char *pattern, const char *haystack, int pos) {
#if (GLIB2_VERSION >= 21400)
	GRegex *re = NULL;
	GMatchInfo *match_info = NULL;
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
#else
	pcre *re;
	const char *error;
	int rc, erroffset, i;
	int ovector[30];
	const char *strptr;
	char *value;

	re = pcre_compile(pattern, PCRE_CASELESS, &error, &erroffset, NULL);
	if(re == NULL) {
		syslog(LOG_NOTICE, "pcre_match : failed to compile pattern %s", pattern);
	} else {
		rc = pcre_exec(re, NULL, haystack, strlen(haystack), 0, 0, ovector, 30);
		if(rc > 0) {
			pcre_get_substring(haystack,ovector,rc,pos,&strptr);
			value = g_strdup((char *)strptr);
		} else {
			syslog(LOG_ERR, "pcre_match : failed to match pattern %s : code was %d", pattern, rc);
		}
	}
	if (strptr != NULL)
		free((char *) strptr);

	return value;
#endif
}

GMimeMessage *get_message(char *msg_path) {
	GMimeMessage *message = NULL;
	GMimeParser *parser;
	GMimeStream *stream;
	int fd;
	
	g_mime_init(0);
	
	if ((fd = open (msg_path, O_RDONLY)) == -1)
		return message;

	stream = g_mime_stream_fs_new(fd);
	parser = g_mime_parser_new_with_stream(stream);
	g_object_unref (stream);

	message = g_mime_parser_construct_message(parser);
	close(fd);
	return message;
}

const char *get_header(char *msg_path, char *header_name) {
	GMimeMessage *message;
	const char *header_value = NULL;
#ifdef GMIME24
	/* g_mime_message_get_header was renamed to 
	 * g_mime_object_get_header in 2.3
	 */
	GMimeObject *object;
	
	message = get_message(msg_path);
	if (message!=NULL) {
		header_value = g_mime_object_get_header(GMIME_OBJECT(message),header_name);
	}
#else
	message = get_message(msg_path);
	if (message!=NULL) {
		header_value = g_mime_message_get_header(message,header_name);
	}
#endif
	 
	g_object_unref(message);
	return header_value;
}

int set_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	GMimeStream *stream;
	GMimeObject *object;
	int fd;
	
	g_mime_init(0);
	message = get_message(msg_path);	
	
	if (message!=NULL) {
		g_mime_object_set_header((GMimeObject *) message,header_name,header_value);
		if ((fd = open (msg_path, O_WRONLY)) == -1)
			return -1;

		stream = g_mime_stream_fs_new(fd);
		g_mime_object_write_to_stream((GMimeObject *) message,stream);
		g_mime_stream_reset(stream);
		
		g_object_unref (stream);
		close(fd);
	} else 
		return -1;

	return 0;
}
