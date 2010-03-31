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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gmodule.h>
#include <unistd.h>
#include <gmime/gmime.h>
#include <fcntl.h>

#include "spmfilter_config.h"
#include "smf_core.h"
#include "smf_modules.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_session.h"
#include "smf_platform.h"
#include "smf_lookup.h"
#include "smf_message_private.h"

#define THIS_MODULE "pipe"

/* copy headers from message object to own GMimeHeaderList */
static void copy_header_func(const char *name, const char *value, gpointer data) {
#ifdef HAVE_GMIME24
	g_mime_header_list_append((GMimeHeaderList *)data,
			g_strdup(name),g_strdup(value));
#else
	g_mime_header_add((GMimeHeader *)data,
			g_strdup(name),g_strdup(value));
#endif
}

/* error handler used when building module queue
 * return 1 if processing should continue, else 0
 */
static int handle_q_error(void *args) {
	SMFSettings_T *settings = smf_settings_get();

	switch (settings->module_fail) {
		case 1:
			return(1);
		default:
			return(0);
	}
}

/* handle processing errors when running queue
 */
static int handle_q_processing_error(int retval, void *args) {
	SMFSettings_T *settings = smf_settings_get();

	if (retval == -1) {
		switch (settings->module_fail) {
			case 1: 
				return(1);
			default:
				return(0);
		}
	} else if(retval == 1) {
		return(1);
	}

	/* if none of the above matched, halt processing, this is just
	 * for safety purposes
	 */
	TRACE(TRACE_DEBUG, "no conditional matched, will stop queue processing!");
	return(0);
}


/* handle nexthop delivery error */
static int handle_nexthop_error(void *args) {
	return(0);
}

int load_modules(void) {
	int ret;
	ProcessQueue_T *q;
	SMFSession_T *session = smf_session_get();

	/* initialize the modules queue handler */
	q = smf_modules_pqueue_init(
		handle_q_error,
		handle_q_processing_error,
		handle_nexthop_error
	);

	if(q == NULL) {
		return(-1);
	}

	/* now tun the process queue */
	ret = smf_modules_process(q,session);
	free(q);

	if(ret != 0) {
		TRACE(TRACE_DEBUG, "pipe engine failed to process modules!");
		return(-1);
	}


	return(0);
}

int load(void) {
	GIOChannel *in;
	GMimeStream *out;
	GMimeObject *message;
	GMimeParser *parser;
	gchar *line;
	gsize length;
	int fd;
	GError *error = NULL;
#ifdef HAVE_GMIME24
	GMimeHeaderList *headers;
#else
	GMimeHeader *headers;
#endif
	SMFSession_T *session = smf_session_get();
	
	smf_core_gen_queue_file(&session->queue_file);

	TRACE(TRACE_DEBUG,"using spool file: '%s'", session->queue_file);
		
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	g_io_channel_set_encoding(in, NULL, NULL);

	if ((fd = open(session->queue_file,O_RDWR|O_CREAT)) == -1) {
		TRACE(TRACE_ERR,"failed writing queue file");
		return -1;
	}

	out = g_mime_stream_fs_new(fd);


	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (g_mime_stream_write(out,line,length) == -1) {
			TRACE(TRACE_ERR,"%s",error->message);
			g_io_channel_unref(in);
			g_object_unref(out);
			close(fd);
			g_free(line);
			remove(session->queue_file);
			g_error_free(error);
			return -1;
		}
		session->msgbodysize+=strlen(line);
		g_free(line);
	}
	
	g_io_channel_unref(in);

	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->msgbodysize);
	session->envelope_to_num = 0;
	
	/* parse email data and fill session struct*/
	/* extract message headers */
	g_mime_stream_flush(out);
	g_mime_stream_seek(out,0,0);
	parser = g_mime_parser_new_with_stream(out);
	message = GMIME_OBJECT(g_mime_parser_construct_message(parser));
	
	smf_message_extract_addresses(message);

#ifdef HAVE_GMIME24
	headers = (void *)g_mime_object_get_header_list(message);
	session->headers = (void *)g_mime_header_list_new();
	g_mime_header_list_foreach(headers, copy_header_func, session->headers);
#else
	headers = (void *)g_mime_object_get_headers(message);
	session->headers = (void *)g_mime_header_new();
	g_mime_header_foreach(headers, copy_header_func, session->headers);
#endif

	g_object_unref(parser);
	g_object_unref(message);
	g_object_unref(out);
	g_io_channel_unref(in);

	close(fd);


	if (load_modules() != 0) {
		remove(session->queue_file);
		smf_session_free();
		TRACE(TRACE_DEBUG,"removing spool file %s",session->queue_file);
		return -1;
	} else {
		remove(session->queue_file);
		smf_session_free();
		TRACE(TRACE_DEBUG,"removing spool file %s",session->queue_file);
		return 0;
	}
}
