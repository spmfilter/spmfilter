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
#include "smf_message.h"
#include "smf_modules.h"

#define THIS_MODULE "message"

/** Copy the current message to disk
 *
 * \param path for the new message file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_message_copy_to_disk(char *path) {
	SMFSession_T *session = smf_session_get();
	GIOChannel *in;
	GMimeStream *out;
	int fd;
	gchar *line;
	GError *error = NULL;
	gboolean header_done = FALSE;

	if ((fd = open(path,O_WRONLY|O_CREAT)) == -1) {
		TRACE(TRACE_ERR,"failed opening destination file");
		return -1;
	}

	out = g_mime_stream_fs_new(fd);

	if (session->is_dirty) {
#ifdef HAVE_GMIME24
		if (g_mime_header_list_write_to_stream((GMimeHeaderList *)session->headers,out) == -1) {
			TRACE(TRACE_ERR,"failed writing file");
			close(fd);
			g_object_unref(out);
			return -1;
		}
#else
		if (g_mime_header_write_to_stream((GMimeHeader *)session->headers,out) == -1) {
			TRACE(TRACE_ERR,"failed writing file");
			close(fd);
			g_object_unref(out);
			return -1;
		}
#endif
	} else {
		header_done = TRUE;
	}

	if ((in = g_io_channel_new_file(session->queue_file,"r", &error)) == NULL) {
		TRACE(TRACE_ERR,"%s",error->message);
		g_error_free(error);
		close(fd);
		g_object_unref(out);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);

	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, "\r\n")==0)||(g_ascii_strcasecmp(line, "\n")==0))
			header_done = TRUE;

		if (header_done) {
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
		} else
			continue;
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
const char *smf_message_header_get(const char *header_name) {
	SMFSession_T *session = smf_session_get();
	const char *header_value = NULL;
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
int smf_message_header_remove(char *header_name) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	if (g_mime_header_list_remove((GMimeHeaderList *)session->headers,header_name)) {
		session->is_dirty = 1;
		return 0;
	}
#else
	if (g_mime_header_remove((GMimeHeader *)session->headers,header_name)) {
		session->is_dirty = 1;
		return 0;
	}
#endif
	return -1;
}

/** Prepends a header. If value is NULL, a space will be set aside for it
 * (useful for setting the order of headers before values can be obtained
 * for them) otherwise the header will be unset.
 *
 * \param header_name name of the header
 * \param header_value new value for the header
 */
void smf_message_header_prepend(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_prepend((GMimeHeaderList *)session->headers,header_name,header_value);
	session->is_dirty = 1;
#else
	g_mime_header_prepend((GMimeHeader *)session->headers,header_name,header_value);
	session->is_dirty = 1;
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
void smf_message_header_append(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)session->headers,header_name,header_value);
	session->is_dirty = 1;
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
void smf_message_header_set(char *header_name, char *header_value) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_set((GMimeHeaderList *)session->headers,header_name,header_value);
	session->is_dirty = 1;
#else
	g_mime_header_set((GMimeHeader *)session->headers,header_name,header_value);
	session->is_dirty = 1;
#endif
	return;
}

/** Allocates a string buffer containing the raw rfc822 headers.
 *
 * \returns a string containing the header block.
 */
char *smf_message_header_to_string(void) {
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
void smf_message_header_foreach(SMFHeaderForeachFunc func, void *user_data) {
	SMFSession_T *session = smf_session_get();

#ifdef HAVE_GMIME24
	g_mime_header_list_foreach((GMimeHeaderList *)session->headers,func,user_data);
#else
	g_mime_header_foreach((GMimeHeader *)session->headers,func,user_data);
#endif
}
