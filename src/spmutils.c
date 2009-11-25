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

/* 
 * Generate a new queue file name
 */
char *gen_queue_file(void) {
	char *tempname = NULL;
	
	/* create spooling file */
	tempname = g_strdup_printf("%s/spmfilter.XXXXXX",QUEUE_DIR);
	if(g_mkstemp(tempname) == -1)
		return NULL;
	
	return tempname;	
}

/*
 * Parse a message file on disk and return a GMimeMessage object
 *
 * msg_path: path to message file
 */
static GMimeMessage *parse_message(char *msg_path) {
	GMimeMessage *message = NULL;
	GMimeParser *parser;
	GMimeStream *stream;
	int fd;

	if ((fd = open(msg_path, O_RDONLY)) == -1) {
		syslog(LOG_ERR, "Cannot open message `%s': %s\n", msg_path, strerror (errno));
		return NULL;
	}
	
	stream = g_mime_stream_fs_new(dup(fd));
	parser = g_mime_parser_new_with_stream (stream);
	g_object_unref(stream);
	message = g_mime_parser_construct_message (parser);
	g_object_unref (parser);
	close(fd);
	return message;
}

/*
 * Write a message to disk
 *
 * msg_path: path for the new message file
 * message: GMimeMessage object
 */
int write_message(char *msg_path, GMimeMessage *message) {
	GMimeStream *stream;
	int fd;
	
	if ((fd = open(msg_path, O_CREAT|O_WRONLY)) == -1) {
		syslog(LOG_ERR, "Cannot open message `%s': %s", msg_path, strerror(errno));
		return -1;
	}
	
	stream = g_mime_stream_fs_new(fd);
	if (g_mime_object_write_to_stream((GMimeObject *) message, stream) == -1) {
		return -1;
	}

	if (g_mime_stream_flush(stream) != 0) {
		return -1;
	}
	g_mime_stream_close(stream);
	g_object_unref(stream);
	close(fd);
	return 0;
}

/* 
 * Gets the value of the requested header if it exists or NULL otherwise.
 *
 * msg_path: path to message or queue file
 * header_name: name of the wanted header
 */
const char *get_header(char *msg_path, char *header_name) {
	GMimeMessage *message;
	const char *header_value = NULL;
	
	message = parse_message(msg_path);
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		header_value = g_mime_object_get_header(GMIME_OBJECT(message),header_name);
#else
		header_value = g_mime_message_get_header(message,header_name);
#endif
	}
	
	g_object_unref(message);
	return header_value;
}

/*
 * Adds an arbitrary header to the message, returns 0 on success.
 * 
 * msg_path: path to message or queue file
 * header_name: name of the new header
 * header_value: value for the new header
 */
int add_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);
	
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_append_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_add_header(message,header_name,header_value);
#endif
		tmp_file = gen_queue_file();
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	
	g_object_unref(message);
	return 0;
}

/*
 * Sets an arbitrary header, returns 0 on success.
 * 
 * msg_path: path to message or queue file
 * header_name: name of the header
 * header_value: new value for the header
 */
int set_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);
	
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_set_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_set_header(message,header_name,header_value);
#endif
		tmp_file = gen_queue_file();
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	
	g_object_unref(message);
	return 0;
}

/*
 * Removed the specified header if it exists, returns 0 on success
 *
 * msg_path: path to message or queue_file
 * header_name: name of the header
 */
int remove_header(char *msg_path, char *header_name) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	message = parse_message(msg_path);
	
	if (message!=NULL) {
		g_mime_object_remove_header((GMimeObject *)message,header_name);
		tmp_file = gen_queue_file();
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	
	g_object_unref(message);
	return 0;
}
