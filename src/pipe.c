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


#include "spmfilter.h"
#include "mailconn.h"
#include "platform.h"

#define THIS_MODULE "pipe"

#define EMAIL_EXTRACT "(?:.*<)?([^>]*)(?:>)?"

typedef int (*LoadMod) (MailConn_T *mconn);

MailConn_T *mconn = NULL;

int load_modules(void) {
	GModule *module;
	LoadMod load_module;
	int i, ret;
	Message_T *msg = NULL;
	Settings_T *settings = get_settings();
	
	for(i = 0; settings->modules[i] != NULL; i++) {
		gchar *path;
		TRACE(TRACE_DEBUG,"loading module %s",settings->modules[i]);

		path = smf_build_module_path(LIB_DIR, settings->modules[i]);
		if(path == NULL) {
			TRACE(TRACE_DEBUG, "failed to build module path for %s", settings->modules[i]);
			return(-1);
		}

		module = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!module) {
			TRACE(TRACE_DEBUG,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return -1;
		}

		if (!g_module_symbol (module, "load", (gpointer *)&load_module)) {
			TRACE(TRACE_DEBUG,"%s", g_module_error());
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return -1;
		}

		ret = load_module(mconn);
		if (ret == -1) {
			g_module_close (module);
			switch (settings->module_fail) {
				case 1: continue;
				default: return -1;
			}
			return -1;
		} else if (ret == 1) {
			g_module_close(module);
			break;
		}

		if (!g_module_close(module))
			TRACE(TRACE_ERR,"%s", g_module_error());
	
		g_free(path);
	}
	
	if (settings->nexthop != NULL ) {
		msg = g_slice_new(Message_T);
		msg->from = g_strdup(mconn->from->addr);
		msg->rcpts = malloc(sizeof(msg->rcpts[mconn->num_rcpts]));
		for (i = 0; i < mconn->num_rcpts; i++) {
			msg->rcpts[i] = calloc(strlen(mconn->rcpts[i]->addr), sizeof(char));
			msg->rcpts[i] = g_strdup(mconn->rcpts[i]->addr);
		}
		msg->num_rcpts = mconn->num_rcpts;
		msg->message_file = g_strdup(mconn->queue_file);
		msg->nexthop = g_strup(settings->nexthop);
		if (smtp_delivery(msg) != 0) {
			TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
			return -1;
		}
		for (i = 0; i < mconn->num_rcpts; i++) {
			g_free(msg->rcpts[i]);
		}
		g_free(msg->from);
		g_free(msg->message_file);
		g_free(msg->nexthop);
		g_slice_free(Message_T,msg);
	}
	return 0;
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
	int i;
	Settings_T *settings = get_settings();
	
	mconn = mconn_new();

	gen_queue_file(&mconn->queue_file);

	TRACE(TRACE_DEBUG,"using spool file: '%s'", mconn->queue_file);
		
	/* start receiving data */
	in = g_io_channel_unix_new(STDIN_FILENO);
	if ((out = g_io_channel_new_file(mconn->queue_file,"w", &error)) == NULL) {
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
			remove(mconn->queue_file);
			g_error_free(error);
			return -1;
		}
		mconn->msgbodysize+=strlen(line);
		g_mime_stream_write_string(gmin,line);
		g_free(line);
	}
	
	g_io_channel_shutdown(out,TRUE,NULL);
	g_io_channel_unref(in);
	g_io_channel_unref(out);

	TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)mconn->msgbodysize);
	mconn->num_rcpts = 0;
	
	/* parse email data and fill mconn struct*/
	g_mime_stream_seek(gmin,0,0);
	parser = g_mime_parser_new_with_stream (gmin);
	g_object_unref(gmin);
	message = g_mime_parser_construct_message (parser);
	mconn->from = g_slice_new(EmailAddress_T);
	mconn->from->addr = get_substring(EMAIL_EXTRACT,g_mime_message_get_sender(message),1);

	TRACE(TRACE_DEBUG,"mconn->from: %s",mconn->from->addr);
	
	mconn->from->is_local = lookup_user(mconn->from->addr);
	TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->from->addr,mconn->from->is_local);
	

	
#ifdef HAVE_GMIME24
	/* g_mime_message_get_all_recipients() appeared in gmime 2.2.5 */
	ia = g_mime_message_get_all_recipients(message);
	for (i=0; i < internet_address_list_length(ia); i++) {
		addr = internet_address_list_get_address(ia,i);
		mconn->rcpts = malloc(sizeof(mconn->rcpts[mconn->num_rcpts]));
		mconn->rcpts[mconn->num_rcpts] = malloc(sizeof(*mconn->rcpts[mconn->num_rcpts]));
		mconn->rcpts[mconn->num_rcpts]->addr = get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
		TRACE(TRACE_DEBUG,"mconn->rcpts[%d]: %s",mconn->num_rcpts, mconn->rcpts[mconn->num_rcpts]->addr);
		mconn->rcpts[mconn->num_rcpts]->is_local = lookup_user(mconn->rcpts[mconn->num_rcpts]->addr);
		TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->rcpts[mconn->num_rcpts]->addr,mconn->rcpts[mconn->num_rcpts]->is_local);
		mconn->num_rcpts++;
	}
#else
	ia = (InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_TO);
	internet_address_list_concat(ia,
		(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_CC));
	internet_address_list_concat(ia,
		(InternetAddressList *)g_mime_message_get_recipients(message,GMIME_RECIPIENT_TYPE_BCC)); 
	while(ia) {
		addr = internet_address_list_get_address(ia);
		mconn->rcpts = malloc(sizeof(mconn->rcpts[mconn->num_rcpts]));
		mconn->rcpts[mconn->num_rcpts] = malloc(sizeof(*mconn->rcpts[mconn->num_rcpts]));
		mconn->rcpts[mconn->num_rcpts]->addr = get_substring(EMAIL_EXTRACT, internet_address_to_string(addr,TRUE),1);
		TRACE(TRACE_DEBUG,"mconn->rcpts[%d]: %s",mconn->num_rcpts, mconn->rcpts[mconn->num_rcpts]->addr);
		mconn->rcpts[mconn->num_rcpts]->is_local = lookup_user(mconn->rcpts[mconn->num_rcpts]->addr);
		TRACE(TRACE_DEBUG,"[%s] is local [%d]", mconn->rcpts[mconn->num_rcpts]->addr,mconn->rcpts[mconn->num_rcpts]->is_local);
		mconn->num_rcpts++;
		ia = internet_address_list_next(ia);
	}
#endif

	g_mime_shutdown();
	if (load_modules() != 0) {
		remove(mconn->queue_file);
		mconn_free(mconn);
		TRACE(TRACE_DEBUG,"removing spool file %s",mconn->queue_file);
		return -1;
	} else {
		remove(mconn->queue_file);
		mconn_free(mconn);
		TRACE(TRACE_DEBUG,"removing spool file %s",mconn->queue_file);
		return 0;
	}
}
