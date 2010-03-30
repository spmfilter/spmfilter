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
#include "smf_message_private.h"
#include "smf_trace.h"
#include "smf_modules.h"

#define THIS_MODULE "session"

SMFSession_T *session = NULL;

/** Initialize SMFSession_T structure
 *
 * \returns pointer to SMFSession_T type
 */
SMFSession_T *smf_session_get(void) {
	if (session == NULL) {
		TRACE(TRACE_DEBUG,"initialize session data");
		session = g_slice_new(SMFSession_T);
		session->helo = NULL;
		session->envelope_from = NULL;
		session->queue_file = NULL;
		session->envelope_to = NULL;
		session->xforward_addr = NULL;
		session->dirty_headers = NULL;
		session->msgbodysize = 0;
		session->headers = NULL;
		session->dirty_headers = NULL;
		session->message_from = NULL;
		session->message_to = NULL;
	}
	
	return session;
}

/** Free SMFSession_T structure
 *
 * \param session SMFSession_T type
 */
void smf_session_free(void) {
	int i;
	TRACE(TRACE_DEBUG,"destroy session data");
	g_free(session->queue_file);
	g_free(session->helo);
	g_free(session->xforward_addr);

	if (session->headers != NULL) {
#ifdef HAVE_GMIME24
		g_mime_header_list_destroy((GMimeHeaderList *)session->headers);
#else
		g_mime_header_destroy((GMimeHeader *)session->headers);
#endif
	}

	if (session->envelope_from != NULL) {
		if (session->envelope_from->addr != NULL) {
			g_free(session->envelope_from->addr);
			smf_lookup_result_free(session->envelope_from->user_data);
		}
		g_slice_free(SMFEmailAddress_T,session->envelope_from);
	}
	
	if (session->envelope_to != NULL) {
		for (i = 0; i < session->envelope_to_num; i++) {
			if (session->envelope_to[i] != NULL) {
				g_free(session->envelope_to[i]->addr);
				smf_lookup_result_free(session->envelope_to[i]->user_data);
				g_slice_free(SMFEmailAddress_T,session->envelope_to[i]);
			}
		}
		g_free(session->envelope_to);
	}

	if (session->message_from != NULL) {
		if (session->message_from->addr != NULL) {
			g_free(session->message_from->addr);
			smf_lookup_result_free(session->message_from->user_data);
		}
		g_slice_free(SMFEmailAddress_T,session->message_from);
	}

	if (session->message_to != NULL) {
		for (i = 0; i < session->message_to_num; i++) {
			if (session->message_to[i] != NULL) {
				g_free(session->message_to[i]->addr);
				smf_lookup_result_free(session->message_to[i]->user_data);
				g_slice_free(SMFEmailAddress_T,session->message_to[i]);
			}
		}
		g_free(session->message_to);
	}

	if (session->dirty_headers != NULL)
		g_slist_free((GSList *)session->headers);
	g_slice_free(SMFSession_T,session);
	session = NULL;
}

/** Copy the current message to disk
 *
 * \param path for the new message file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_to_file(char *path) {
	SMFSession_T *session = smf_session_get();
	GIOChannel *in;
	GMimeStream *out;
	int fd;
	gchar *line;
	GError *error = NULL;

	if (path == NULL)
		return -1;

	if ((fd = open(path,O_WRONLY|O_CREAT,S_IRWXU)) == -1) {
		TRACE(TRACE_ERR,"failed opening destination file");
		return -1;
	}

	out = g_mime_stream_fs_new(fd);

	if (smf_modules_flush_dirty(session) != 0)
		TRACE(TRACE_ERR,"message flush failed");

	if ((in = g_io_channel_new_file(session->queue_file,"r", &error)) == NULL) {
		TRACE(TRACE_ERR,"%s",error->message);
		g_error_free(error);
		close(fd);
		g_object_unref(out);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);

	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (g_mime_stream_write(out,line,strlen(line)) == -1) {
			TRACE(TRACE_ERR,"failed writing file");
			g_io_channel_shutdown(in,TRUE,NULL);
			g_io_channel_unref(in);
			close(fd);
			g_object_unref(out);
			g_free(line);
			g_remove(path);
			return -1;
		}
		g_free(line);
	}

	g_mime_stream_flush(out);
	close(fd);
	g_object_unref(out);
	g_io_channel_shutdown(in,TRUE,NULL);
	g_io_channel_unref(in);

	return 0;
}

/** Gets the value of the first header with the name requested.
 *
 * \param header_name name of the wanted header
 *
 * \returns requested header
 */
const char *smf_session_header_get(const char *header_name) {
	SMFSession_T *session = smf_session_get();
	const char *header_value = NULL;

	while (session->dirty_headers) {
		SMFHeaderModification_T *mod = (SMFHeaderModification_T *)((GSList *)session->dirty_headers)->data;
		if (g_ascii_strcasecmp(mod->name,header_name) == 0)
			return mod->value;
		session->dirty_headers = ((GSList *)session->dirty_headers)->next;
	}
#ifdef HAVE_GMIME24
	header_value = g_mime_header_list_get((GMimeHeaderList *)session->headers,header_name);
#else
	header_value = g_mime_header_get((GMimeHeader *)session->headers,header_name);
#endif
	return header_value;
}

/** Removed the specified header if it exists
 *
 * \param header_name name of the header
 *
 * \returns 0 on success or -1 in case of error
 */
void smf_session_header_remove(char *header_name) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_REMOVE;
	header->name = g_strdup(header_name);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_remove((GMimeHeaderList *)session->headers,header_name);
#else
	g_mime_header_remove((GMimeHeader *)session->headers,header_name);
#endif
	return;
}

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_prepend(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_PREPEND;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_prepend((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	g_mime_header_prepend((GMimeHeader *)session->headers,header_name,header_value);
#endif
	return;
 }

/** Appends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_append(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_APPEND;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	TRACE(TRACE_WARNING,"function not implemented in GMime < 2.4");
#endif
	return;
}

/** Set the value of the specified header. If value is NULL and the header,
 * name, had not been previously set, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * Note: If there are multiple headers with the specified field name,
 * the first instance of the header will be replaced and further instances
 * will be removed.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_session_header_set(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();
	SMFHeaderModification_T *header = g_slice_new(SMFHeaderModification_T);
	header->status = HEADER_SET;
	header->name = g_strdup(header_name);
	header->value = g_strdup(header_value);
	session->dirty_headers = (void *) g_slist_append((GSList *)session->dirty_headers,header);

#ifdef HAVE_GMIME24
	g_mime_header_list_set((GMimeHeaderList *)session->headers,header_name,header_value);
#else
	g_mime_header_set((GMimeHeader *)session->headers,header_name,header_value);
#endif
	return;
}

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \returns a string containing the header block.
 */
char *smf_session_header_to_string(void) {
	SMFSession_T *session = smf_session_get();
	char *header = NULL;

#ifdef HAVE_GMIME24
	header = g_mime_header_list_to_string((GMimeHeaderList *)session->headers);
#else
	header = g_mime_header_to_string((GMimeHeader *)session->headers);
#endif
	return header;
}

/** Calls func for each header name/value pair.
 *
 * \param func function to be called for each header.
 * \param user_data user data to be passed to the func.
 */
void smf_session_header_foreach(SMFHeaderForeachFunc func, void *user_data) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_foreach((GMimeHeaderList *)session->headers,func,user_data);
#else
	g_mime_header_foreach((GMimeHeader *)session->headers,func,user_data);
#endif
}

/** Prepend text to subject
 *
 * \param text text to prepend
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_session_subject_prepend(char *text) {
	char *subject = (char *)smf_session_header_get("subject");
	if (subject == NULL)
		return -1;

	smf_session_header_set("subject",g_strdup_printf("%s %s",text,subject));
	return 0;
}

/** Append text to subject
 *
 * \param text text to append
 *
 * \return 0 on success or -1 in case of error
 */
int smf_session_subject_append(char *test) {
	char *subject = (char *)smf_session_header_get("subject");

	smf_session_header_set("subject",g_strdup_printf("%s %s",subject, test));
	return 0;
}

/** Retrieve a SMFMessage_T object from the
 *  current session.
 *
 * \returns SMFMessage_T object
 */
SMFMessage_T *smf_session_get_message(void) {
	SMFSession_T *session = smf_session_get();
	SMFMessage_T *message;
	GMimeStream *stream, *mem_stream;
	GMimeParser *parser;
	int fd;

	message = smf_message_new();

	if ((fd = open(session->queue_file,O_RDONLY)) == -1) {
		return NULL;
	}

	stream = g_mime_stream_fs_new(fd);
	
	mem_stream = g_mime_stream_mem_new();
	g_mime_stream_write_to_stream(stream,mem_stream);

	g_mime_stream_seek(mem_stream,0,0);

	parser = g_mime_parser_new_with_stream(mem_stream);
	message->data = g_mime_parser_construct_message(parser);

	g_object_unref(parser);
	g_mime_stream_close(stream);
	g_object_unref(stream);
	close(fd);
	g_object_unref(mem_stream);


	return message;
}
