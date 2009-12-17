#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef HAVE_PCRE
#include <pcre.h>
#endif
#include <gmime/gmime.h>

#include "spmfilter.h"

#define THIS_MODULE "spmutils"

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

/* 
 * Generate a new queue file name
 */
int gen_queue_file(char **tempname) {
	/* create spooling file */
	*tempname = g_strdup_printf("%s/spmfilter.XXXXXX",settings->queue_dir);
	if(g_mkstemp(*tempname) == -1)
		return -1;
	
	return 0;	
}

/*
 * Parse a message file on disk and return a GMimeMessage object
 *
 * msg_path: path to message file
 */
GMimeMessage *parse_message(char *msg_path) {
	GMimeMessage *message = NULL;
	GMimeParser *parser;
	GMimeStream *stream;
	int fd;

	if ((fd = g_open(msg_path, O_RDONLY)) == -1) {
		TRACE(TRACE_ERR, "cannot open message `%s': %s", msg_path, strerror(errno));
		return NULL;
	}
	
	stream = g_mime_stream_fs_new(dup(fd));
	parser = g_mime_parser_new_with_stream(stream);
	g_object_unref(stream);
	message = g_mime_parser_construct_message(parser);
	g_object_unref(parser);
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
	
	if ((fd = g_open(msg_path, O_CREAT|O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH)) == -1) {
		TRACE(TRACE_ERR, "cannot open message `%s': %s", msg_path, strerror(errno));
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

	g_free(header_name);
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
		gen_queue_file(&tmp_file);
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_free(header_value);
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
		gen_queue_file(&tmp_file);
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_free(header_value);
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
		gen_queue_file(&tmp_file);
		
		if (write_message(tmp_file,message) != 0) 
			return -1;
	} else 
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_object_unref(message);
	return 0;
}

/*
 * Generates a unique maildir filename
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

/* expands a query string */
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
	*buf = (char *)calloc(512, sizeof(char));
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
