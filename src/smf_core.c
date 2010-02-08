/*
 * file: smf_core.h
 * desc: implementation of core routines used by smf
 * auth: Sebastian Jaekel <sj@space.net>
 */

#include <stdio.h>
#include <stdlib.h>
#include <glib-2.0/gmodule.h>

#include "spmfilter.h"
#include "smf_core.h"

#define THIS_MODULE "smf_core"


ProcessQueue_T *smf_core_pqueue_init(int(*loaderr)(void *args),
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

int smf_core_process_modules(ProcessQueue_T *q, MailConn_T *mconn) {
	int i;
	int retval;
	ModuleLoadFunction runner;
	gchar *path;
	GModule *mod;
	gpointer *sym;
	Settings_T *settings = get_settings();

	for(i=0;settings->modules[i] != NULL; i++) {
		path = smf_build_module_path(LIB_DIR, settings->modules[i]);
		if(path == NULL) {
			TRACE(TRACE_DEBUG, "failed to build module path for %s", settings->modules[i]);
			return(-1);
		}

		mod = g_module_open(path, G_MODULE_BIND_LAZY);
		if (!mod) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());

			if(q->load_error(NULL) == 0)
				return(-1);
		}

		if (!g_module_symbol(mod, "load", sym)) {
			g_free(path);
			TRACE(TRACE_ERR,"%s", g_module_error());

			if(q->load_error(NULL) == 0)
				return(-1);
		}

		/* cast spell and execute */
		runner = (ModuleLoadFunction)sym;
		retval = runner(mconn);

		/* clean up */
		g_free(path);
		g_module_close(mod);

		if(retval != 0) {
			/* FIXME: dont forget to close the gmodule before leaving */
			retval = q->processing_error(retval, NULL);

			if(retval == 0) {
				return(-1);
			} else if(retval == 1) {
				continue;
			} else if(retval == 2) {
				break;
			}
		}
	}

	/* queue is done, if we're still here check for next hop and
	 * deliver
	 */
	if (settings->nexthop != NULL ) {
		return(smf_core_deliver_nexthop(q, mconn));
	}

	return(0);
}


int smf_core_deliver_nexthop(ProcessQueue_T *q,MailConn_T *mconn) {
	int i;
	Message_T *msg;
	Settings_T *settings = get_settings();

	msg = g_slice_new(Message_T);
	msg->from = g_strdup(mconn->from->addr);

	/* allocate memory for recipients */
	msg->rcpts = g_malloc(sizeof(msg->rcpts[mconn->num_rcpts]));
	if(msg->rcpts == NULL) {
		TRACE(TRACE_ERR, "failed to allocated memory for recipients!");
		return(-1);
	}

	/* copy recipients in place */
	for (i = 0; i < mconn->num_rcpts; i++) {
		msg->rcpts[i] = g_strdup(mconn->rcpts[i]->addr);
	}

	msg->num_rcpts = mconn->num_rcpts;
	msg->message_file = g_strdup(mconn->queue_file);
	msg->nexthop = g_strup(settings->nexthop);

	/* now deliver, if delivery fails, call error hook */
	if (smtp_delivery(msg) != 0) {
		TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
		q->nexthop_error(NULL);
		return(-1);
	}

	/* free all allocated recipient resources */
	for (i = 0; i < mconn->num_rcpts; i++) {
		g_free(msg->rcpts[i]);
	}

	/* free all other message stuff */
	g_free(msg->from);
	g_free(msg->message_file);
	g_free(msg->nexthop);
	g_slice_free(Message_T,msg);

	return(0);
}