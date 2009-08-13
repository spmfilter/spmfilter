#include <string.h>
#include <syslog.h>
#include <glib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
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

char *gen_queue_file(void) {
	char *tempname = NULL;
	
	/* create spooling file */
	tempname = g_strdup_printf("%s/spmfilter.XXXXXX",QUEUE_DIR);
	g_mkstemp(tempname);
	
	return tempname;	
}

static GMimeMessage *parse_message(int fd) {
	GMimeMessage *message = NULL;
	GMimeParser *parser;
	GMimeStream *stream;
	
	stream = g_mime_stream_fs_new (dup (fd));
	parser = g_mime_parser_new_with_stream (stream);
	g_object_unref (stream);
	message = g_mime_parser_construct_message (parser);
	g_object_unref (parser);
	
	return message;
}

int write_message(int fd, GMimeMessage *message) {
	GMimeStream *stream;
	stream = g_mime_stream_fs_new(fd);
	if (g_mime_object_write_to_stream((GMimeObject *) message, stream) == -1) {
		return -1;
	}

	if (g_mime_stream_flush(stream) != 0) {
		return -1;
	}
	g_mime_stream_close(stream);
	g_object_unref(stream);
	
	return 0;
}

const char *get_header(char *msg_path, char *header_name) {
	GMimeMessage *message;
	const char *header_value = NULL;
	int fd;
#ifdef HAVE_GMIME24
	GMimeObject *object;
#endif

	if ((fd = open(msg_path, O_RDONLY)) == -1) {
		syslog(LOG_ERR, "Cannot open message `%s': %s\n", msg_path, strerror (errno));
		return NULL;
	}

#ifdef HAVE_GMIME24
	message = parse_message(fd);
	close (fd);
	if (message!=NULL) {
		header_value = g_mime_object_get_header(GMIME_OBJECT(message),header_name);
	}
#else
	message = parse_message(fd);
	close (fd);
	if (message!=NULL) {
		header_value = g_mime_message_get_header(message,header_name);
	}
#endif
	 
	g_object_unref(message);
	return header_value;
}

int set_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	int fd;
	char *tmp_file;
	
	if ((fd = open(msg_path, O_RDONLY)) == -1) {
		syslog(LOG_ERR, "Cannot open message `%s': %s\n", msg_path, strerror (errno));
		return -1;
	}
	
	message = parse_message(fd);
	close(fd);
	
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_append_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_add_header(message,header_name,header_value);
#endif
		tmp_file = gen_queue_file();
		
		if ((fd = open(tmp_file, O_CREAT|O_WRONLY)) == -1) {
			syslog(LOG_ERR, "Cannot open message `%s': %s", msg_path, strerror(errno));
			return -1;
		}
		
		if (write_message(fd,message) != 0) {
			syslog(LOG_ERR, "Cannot write to stream");
			close(fd);
			return -1;
		}
		close(fd);
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	
	g_object_unref(message);
	return 0;
}
