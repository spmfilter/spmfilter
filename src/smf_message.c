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

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>

#include "spmfilter_config.h"
#include "smf_session.h"
#include "smf_trace.h"
#include "smf_core.h"

#define THIS_MODULE "message"

/** Parse a message file on disk and return a GMimeMessage object
 *
 * \param msg_path path to message file
 *
 * \returns GMimeMessage object
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


/** Write a message to disk
 *
 * \param new_path path for the new message file
 * \param queue_file path of the queue file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_write(char *new_path, char *queue_file) {
	GMimeStream *stream;
	int fd;
	GMimeMessage *message;

	g_mime_init(0);
	message = parse_message(queue_file);

	if ((fd = g_open(new_path, O_CREAT|O_WRONLY, S_IRWXU|S_IRGRP|S_IROTH)) == -1) {
		TRACE(TRACE_ERR, "cannot open message `%s': %s", new_path, strerror(errno));
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
	g_mime_shutdown();
	return 0;
}

/** Gets the value of the requested header if it exists or NULL otherwise.
 *
 * \param header_name name of the wanted header
 *
 * \returns requested header
 */
char *smf_message_get_header(const char *header_name) {
//	SMFSession_T *session = smf_session_get();
//	GMimeStream *stream;
//	GMimeMessage *message;
//	GMimeParser *parser;
//	char *test;
//	char *header_value = NULL;

//	stream = g_mime_stream_mem_new_with_buffer(mconn->header->data,strlen(mconn->header->data) + 1);
//	parser = g_mime_parser_new_with_stream(stream);
//	message = g_mime_parser_construct_message(parser);
//	TRACE(TRACE_DEBUG,"MESSAGE: %s",message);
//	g_object_unref(parser);

//	if (message!=NULL) {
//#ifdef HAVE_GMIME24
//		header_value = (char *)g_mime_object_get_header(GMIME_OBJECT(message),header_name);
//#else
//		header_value = g_mime_message_get_header(message,header_name);
//#endif
//	}
//	TRACE(TRACE_DEBUG,"HEADER VALUE: %s",header_value);
//	g_object_unref(message);
//	g_object_unref(stream);

//	return header_value;
/*
	gchar **lines;
	gchar **header;
	char *header_value = NULL;
	gboolean found = FALSE;
	int i;

	lines = g_strsplit(mconn->header->data,"\n",-1);
	while(lines[i]) {
		if (g_ascii_strncasecmp(lines[i],header_name,strlen(header_name)) == 0) {
			header = g_strsplit(lines[i],":",2);
			found = TRUE;
		}
		i++;
	}

	if (found) {
		header_value = g_strdup(header[1]);
		g_strstrip(header_value);
		g_strfreev(header);
	}
	g_strfreev(lines);

	return header_value;  */
	return NULL;
/*	GMimeMessage *message;
	char *header_value = NULL;

	g_mime_init(0);
	message = parse_message(msg_path);
	if (message!=NULL) {
#ifdef HAVE_GMIME24
		header_value = (char *)g_mime_object_get_header(GMIME_OBJECT(message),header_name);
#else
		header_value = g_mime_message_get_header(message,header_name);
#endif
	}

	g_object_unref(message);
	g_mime_shutdown();
	return header_value;
*/

}

/** Removed the specified header if it exists
 *
 * \param msg_path path to message or queue_file
 * \param header_name name of the header
 * 
 * \returns 0 on success or -1 in case of error
 */
int smf_mesage_remove_header(char *msg_path, char *header_name) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	g_mime_init(0);
	message = parse_message(msg_path);
	
	if (message!=NULL) {
		g_mime_object_remove_header((GMimeObject *)message,header_name);
		smf_core_gen_queue_file(&tmp_file);
		
		if (smf_message_write(tmp_file,msg_path) != 0) {
			g_mime_shutdown();
			return -1;
		}
	} else {
		g_mime_shutdown();
		return -1;
	}

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_object_unref(message);
	g_mime_shutdown();
	return 0;
}

/** Sets an arbitrary header
 *
 * \param msg_path path to message or queue file
 * \param header_name name of the header
 * \param header_value new value for the header
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_set_header(char *msg_path, char *header_name, char *header_value) {
	GMimeMessage *message = NULL;
	char *tmp_file;

	g_mime_init(0);
	message = parse_message(msg_path);

	if (message!=NULL) {
#ifdef HAVE_GMIME24
		g_mime_object_set_header((GMimeObject *) message,header_name,header_value);
#else
		g_mime_message_set_header(message,header_name,header_value);
#endif
		smf_core_gen_queue_file(&tmp_file);

		if (smf_message_write(tmp_file,msg_path) != 0)
			return -1;
	} else
		return -1;

	g_remove(msg_path);
	g_rename(tmp_file,msg_path);
	g_free(tmp_file);
	g_free(header_name);
	g_free(header_value);
	g_object_unref(message);
	g_mime_shutdown();
	return 0;
}
