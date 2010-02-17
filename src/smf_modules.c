/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Sebastian Jaekel, Axel Steiner and SpaceNet AG
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
#include <unistd.h>
#include <fcntl.h>
#include <glib.h>
#include <gmodule.h>
#include <glib/gstdio.h>
#include <gmime/gmime.h>

#include "spmfilter_config.h"
#include "smf_core.h"
#include "smf_session.h"
#include "smf_message.h"
#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_modules.h"
#include "smf_platform.h"

#define THIS_MODULE "smf_modules"


ProcessQueue_T *smf_modules_pqueue_init(int(*loaderr)(void *args),
	int (*processerr)(int retval, void *args),
	int (*nhoperr)(void *args))
{
	ProcessQueue_T *q;

	q = (ProcessQueue_T *)calloc(1, sizeof(ProcessQueue_T));
	if(q == NULL) {
		TRACE(TRACE_ERR, "failed to allocate memory for process queue!");
		return(NULL);
	}

	q->load_error = loaderr;
	q->processing_error = processerr;
	q->nexthop_error = nhoperr;

	return q;
}

/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSession_T *session) {
	gchar *line;
	GIOChannel *in;
	GMimeStream *out;
	int fd;
	GError *error = NULL;
	char *new_queue_file;
	gboolean header_done = FALSE;

	if (session->is_dirty == 0)
		return 0;

	TRACE(TRACE_DEBUG,"flushing header information to filesystem");

	smf_core_gen_queue_file(&new_queue_file);
	if ((fd = open(new_queue_file,O_RDWR|O_CREAT)) == -1) {
		TRACE(TRACE_ERR,"failed writing queue file");
		return -1;
	}

	out = g_mime_stream_fs_new(fd);
#ifdef HAVE_GMIME24
	if (g_mime_header_list_write_to_stream((GMimeHeaderList *)session->headers,out) == -1) {
		TRACE(TRACE_ERR,"failed writing queue file");
		close(fd);
		g_object_unref(out);
		g_remove(new_queue_file);
		return -1;
	}
#else
	if (g_mime_header_write_to_stream((GMimeHeader *)session->headers,out) == -1) {
		TRACE(TRACE_ERR,"failed writing queue file");
		close(fd);
		g_object_unref(out);
		g_remove(new_queue_file);
		return -1;
	}
#endif

	if ((in = g_io_channel_new_file(session->queue_file,"r", &error)) == NULL) {
		TRACE(TRACE_ERR,"%s",error->message);
		g_error_free(error);
		close(fd);
		g_object_unref(out);
		g_remove(new_queue_file);
		return -1;
	}
	g_io_channel_set_encoding(in, NULL, NULL);

	while (g_io_channel_read_line(in, &line, NULL, NULL, NULL) == G_IO_STATUS_NORMAL) {
		if ((g_ascii_strcasecmp(line, "\r\n")==0)||(g_ascii_strcasecmp(line, "\n")==0))
			header_done = TRUE;

		if (header_done) {
			if (g_mime_stream_write(out,line,strlen(line)) == -1) {
				TRACE(TRACE_ERR,"failed writing queue file");
				g_io_channel_shutdown(in,TRUE,NULL);
				g_io_channel_unref(in);
				close(fd);
				g_object_unref(out);
				g_free(line);
				g_remove(new_queue_file);
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

	if (g_remove(session->queue_file) != 0) {
		TRACE(TRACE_ERR,"failed to remove queue file");
		return -1;
	}

	if(g_rename(new_queue_file,session->queue_file) != 0) {
		TRACE(TRACE_ERR,"failed to rename queue file");
		return -1;
	}
	g_free(new_queue_file);

	session->is_dirty = 0;
	return 0;
}

int smf_modules_process(ProcessQueue_T *q, SMFSession_T *session) {
	int i;
	int retval;
	ModuleLoadFunction runner;
	gchar *path;
	GModule *mod;
	gpointer *sym;
	SMFSettings_T *settings = smf_settings_get();

	for(i=0;settings->modules[i] != NULL; i++) {
		path = (gchar *)smf_build_module_path(LIB_DIR, settings->modules[i]);
		if(path == NULL) {
			TRACE(TRACE_DEBUG, "failed to build module path for %s", settings->modules[i]);
			return(-1);
		}

		TRACE(TRACE_DEBUG, "preparing to run module %s", settings->modules[i]);

		mod = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!mod) {
			g_free(path);
			TRACE(TRACE_ERR,"module failed to load : %s", g_module_error());

			if(q->load_error(NULL) == 0)
				return(-1);
			else
				continue;
		}

		if (!g_module_symbol(mod, "load", (gpointer *)&sym)) {
			TRACE(TRACE_ERR,"symbol load could not be foudn : %s", g_module_error());
			g_free(path);
			g_module_close(mod);

			if(q->load_error(NULL) == 0)
				return(-1);
			else
				continue;
		}

		/* cast spell and execute */
		runner = (ModuleLoadFunction)sym;
		retval = runner(session);

		if (smf_modules_flush_dirty(session) != 0)
			TRACE(TRACE_ERR,"message flush failed");

		/* clean up */
		g_free(path);
		g_module_close(mod);

		if(retval != 0) {
			retval = q->processing_error(retval, NULL);

			if(retval == 0) {
				TRACE(TRACE_ERR, "module %s failed to run, will stop processing!",
					settings->modules[i]);
				return(-1);
			} else if(retval == 1) {
				TRACE(TRACE_ERR, "module %s failed to run, will continue with next module!",
					settings->modules[i]);
				continue;
			} else if(retval == 2) {
				TRACE(TRACE_ERR, "module %s stopped processing, will now begin nexthop processing !",
					settings->modules[i]);
				break;
			}
		}

		TRACE(TRACE_DEBUG, "module %s finished successfully", settings->modules[i]);
	}

	TRACE(TRACE_DEBUG, "module processing finished successfully.");

	/* queue is done, if we're still here check for next hop and
	 * deliver
	 */
	if (settings->nexthop != NULL ) {
		TRACE(TRACE_DEBUG, "will now deliver to nexthop %s", settings->nexthop);
		return(smf_modules_deliver_nexthop(q, session));
	}

	return(0);
}

int smf_modules_deliver_nexthop(ProcessQueue_T *q,SMFSession_T *session) {
	int i;
	SMFMessage_T *msg;
	SMFSettings_T *settings = smf_settings_get();

	msg = g_slice_new(SMFMessage_T);
	msg->from = g_strdup(session->from->addr);

	/* allocate memory for recipients */
	msg->rcpts = g_malloc(sizeof(msg->rcpts[session->num_rcpts]));
	if(msg->rcpts == NULL) {
		TRACE(TRACE_ERR, "failed to allocated memory for recipients!");
		return(-1);
	}

	/* copy recipients in place */
	for (i = 0; i < session->num_rcpts; i++) {
		msg->rcpts[i] = g_strdup(session->rcpts[i]->addr);
	}

	msg->num_rcpts = session->num_rcpts;
	msg->message_file = g_strdup(session->queue_file);
	msg->nexthop = g_strup(settings->nexthop);

	/* now deliver, if delivery fails, call error hook */
	if (smf_message_deliver(msg) != 0) {
		TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
		q->nexthop_error(NULL);
		return(-1);
	}

	/* free all allocated recipient resources */
	for (i = 0; i < session->num_rcpts; i++) {
		g_free(msg->rcpts[i]);
	}

	/* free all other message stuff */
	g_free(msg->from);
	g_free(msg->message_file);
	g_free(msg->nexthop);
	g_slice_free(SMFMessage_T,msg);

	return(0);
}
