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
	int (*processerr)(int retval, void *args))
{
	ProcessQueue_T *q;

	q = (ProcessQueue_T *)calloc(1, sizeof(ProcessQueue_T));
	if(q == NULL) {
		TRACE(TRACE_ERR, "failed to allocate memory for process queue!");
		return(NULL);
	}

	q->load_error = loaderr;
	q->processing_error = processerr;
	q->queue = g_ptr_array_new();

	return q;
}

int smf_core_load_modules(ProcessQueue_T *q) {
	int i;
	gchar *path;
	GModule *mod;
	gpointer *sym;
	Settings_T *settings = get_settings();

	/* load every module and save entry pointers into queue */
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
		} else {
			g_ptr_array_add(q->queue, sym);
		}
	}

	return(0);
}

int smf_core_process_modules(ProcessQueue_T *q, MailConn_T *mconn) {
	int i;
	int retval;
	ModuleLoadFunction runner;
	Settings_T *settings = get_settings();

	for(i=0; i < q->queue->len; i++) {
		runner = (ModuleLoadFunction)g_ptr_array_index(q->queue,i);
		retval = runner(mconn);

		if(retval != 0) {
			/* FIXME: dont forget to close the gmodule before leaving */
			if(q->processing_error(retval,NULL) == 0) {
				return(-1);
			}
		}
	}

}