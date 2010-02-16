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

#define THIS_MODULE "pipe"

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"

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
		return(2);
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
		TRACE(TRACE_DEBUG, "smtp engine failed to process modules!");
		return(-1);
	}


	return(0);
}

int load(void) {
	GIOChannel *in, *out;
	GMimeStream *gmin;
	GMimeMessage *message;
	GMimeParser *parser;
	gchar *line;
	gsize length;
	GError *error = NULL;
	InternetAddressList *ia;
	InternetAddress *addr;
	SMFSession_T *session = smf_session_get();
	int i;
	

	smf_core_gen_queue_file(&session->queue_file);

	TRACE(TRACE_DEBUG,"using spool file: '%s'", session->queue_file);
		
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(session->queue_file,"w", &error)) == NULL) {
		TRACE(TRACE_ERR,"%s",error->message);
		g_error_free(error);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);
	g_io_channel_set_encoding(out, NULL, NULL);
	
	/* initialize GMime */
	g_mime_init (0);
	gmin = g_mime_stream_mem_new();

	while (g_io_channel_read_line(in, &line, &length, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if (g_io_channel_write_chars(out, line, -1, &length, &error) != G_IO_STATUS_NORMAL) {
			TRACE(TRACE_ERR,"%s",error->message);
			g_io_channel_shutdown(out,TRUE,NULL);
			g_io_channel_unref(in);
			g_io_channel_unref(out);
			g_free(line);
			remove(session->queue_file);
			g_error_free(error);
			return -1;
		}
		session->msgbodysize+=strlen(line);
		g_mime_stream_write_string(gmin,line);
		g_free(line);
	}
	
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(in);
	g_io_channel_unref(out);

	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->msgbodysize);
	session->num_rcpts = 0;
	
	/* parse email data and fill session struct*/
	g_mime_stream_seek(gmin,0,0);
	parser = g_mime_parser_new_with_stream (gmin);
	g_object_unref(gmin);
	message = g_mime_parser_construct_message (parser);
	session->from = g_slice_new(EmailAddress_T);
	session->from->addr = smf_core_get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(message),1);

	TRACE(TRACE_DEBUG,"session->from: %s",session->from->addr);
	
	session->from->is_local = smf_lookup_check_user(session->from->addr);
	TRACE(TRACE_DEBUG,"[%s] is local [%d]", session->from->addr,session->from->is_local);
	

	
#ifdef HAVE_GMIME24
	/* g_mime_message_get_all_recipients() appeared in gmime 2.2.5 */
	ia = g_mime_message_get_all_recipients(message);
	for (i=0; i < internet_address_list_length(ia); i++) {
		addr = internet_address_list_get_address(ia,i);
		session->rcpts = malloc(sizeof(session->rcpts[session->num_rcpts]));
		session->rcpts[session->num_rcpts] = malloc(sizeof(*session->rcpts[session->num_rcpts]));
		session->rcpts[session->num_rcpts]->addr = smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
		TRACE(TRACE_DEBUG,"session->rcpts[%d]: %s",session->num_rcpts, session->rcpts[session->num_rcpts]->addr);
		session->rcpts[session->num_rcpts]->is_local = smf_lookup_check_user(session->rcpts[session->num_rcpts]->addr);
		TRACE(TRACE_DEBUG,"[%s] is local [%d]", session->rcpts[session->num_rcpts]->addr,session->rcpts[session->num_rcpts]->is_local);
		session->num_rcpts++;
	}
#else
	ia = (InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_TO);
	internet_address_list_concat(ia,
		(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_CC));
	internet_address_list_concat(ia,
		(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_BCC)); 
	while(ia) {
		addr = internet_address_list_get_address(ia);
		session->rcpts = malloc(sizeof(session->rcpts[session->num_rcpts]));
		session->rcpts[session->num_rcpts] = malloc(sizeof(*session->rcpts[session->num_rcpts]));
		session->rcpts[session->num_rcpts]->addr = smf_core_get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
		TRACE(TRACE_DEBUG,"session->rcpts[%d]: %s",session->num_rcpts, session->rcpts[session->num_rcpts]->addr);
		session->rcpts[session->num_rcpts]->is_local = smf_lookup_check_user(session->rcpts[session->num_rcpts]->addr);
		TRACE(TRACE_DEBUG,"[%s] is local [%d]", session->rcpts[session->num_rcpts]->addr,session->rcpts[session->num_rcpts]->is_local);
		session->num_rcpts++;
		ia = internet_address_list_next(ia);
	}
#endif

	g_mime_shutdown();
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
