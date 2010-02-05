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


ProcessQueue_T *smf_core_pqueue_init(void(*errhandler)(void *args)) {
	ProcessQueue_T *q;

	q = (ProcessQueue_T *)calloc(1, sizeof(ProcessQueue_T));
	q->errhandler = errhandler;
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
		if (!mod) 
			goto error;

		if (!g_module_symbol(mod, "load", sym))
			goto error;
	}

	return(0);

	error:
		g_free(path);
		q->errhandler(NULL);
		TRACE(TRACE_ERR,"%s", g_module_error());
		return(-1);
}
