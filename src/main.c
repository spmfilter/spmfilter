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
#include <glib.h>
#include <gmodule.h>
#include <time.h>

#include "spmfilter.h"
#include "smf_settings.h"
#include "smf_lookup.h"

#define THIS_MODULE "spmfilter"

typedef int (*LoadEngine) (void);

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;
	clock_t start_process, stop_process;
	GModule *module;
	LoadEngine load_engine;
	gchar *engine_path;
	int ret;
	Settings_T *settings = smf_settings_get();

	/* all cmd args */
	GOptionEntry entries[] = {
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &settings->debug, "verbose logging", NULL},
		{ "file", 'f', 0, G_OPTION_ARG_STRING, &settings->config_file, "alternate config file", NULL},
		{ NULL }
	};

	/* parse cmd args */
	context = g_option_context_new ("- spmfilter options");
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_option_context_free(context);
		g_print("%s\n", error->message);
		g_error_free(error);
		return 0;
	}

	g_option_context_free(context);

	/* parse config file and fill settings struct */
	if (parse_config() != 0) 
		return -1;

	/* connect to database/ldap server, if necessary */
	if (lookup_connect() != 0) {
		TRACE(TRACE_ERR,"Unable to establish lookup connection!");
		return -1;
	}

	/* check queue dir */
	if (!g_file_test (settings->queue_dir, G_FILE_TEST_EXISTS)) {
		TRACE(TRACE_INFO,"queue directory not available, will create it...");
		
		if(g_mkdir_with_parents(settings->queue_dir,0700)!=0) {
			TRACE(TRACE_ERR,"Could not create queue dir!");
			return -1;
		}
	}

	/* start clock, to see how long
	 * the processing time takes */
	start_process = clock();

	/* check if engine module starts with lib */
	if (g_str_has_prefix(settings->engine,"lib")) {
#ifdef __APPLE__
		engine_path = g_module_build_path(LIB_DIR,g_strdup_printf("%s.dylib",settings->engine));
#else
		engine_path = g_module_build_path(LIB_DIR,settings->engine);
#endif
	} else {
		/* if not prepend lib */
#ifdef __APPLE__
		engine_path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s.dylib",settings->engine));
#else
		engine_path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",settings->engine));
#endif
	}

	/* try to open engine module */
	module = g_module_open(engine_path, G_MODULE_BIND_LAZY);
	if (!module) {
		TRACE(TRACE_ERR,"%s\n", g_module_error());
		return -1;
	}

	/* check if the module provides the function load() */
	if (!g_module_symbol(module, "load", (gpointer *)&load_engine)) {
		TRACE(TRACE_ERR,"%s", g_module_error());
		return -1;
	}

	/* start processing engine */
	ret = load_engine();

	/* processing is done, we can
	 * stop our clock */
	stop_process = clock();
	if (settings->debug) {
		TRACE(TRACE_DEBUG,"processing time: %0.5f sec.", (float)(stop_process-start_process)/CLOCKS_PER_SEC);
	}

	if (lookup_disconnect() != 0)
		TRACE(TRACE_ERR,"Unable to destroy lookup connection!");

	/* free all stuff */
	if (!g_module_close(module))
		TRACE(TRACE_WARNING,"%s", g_module_error());
	g_free(engine_path);
	free_settings(settings);

	if (ret != 0) {
		return -1;
	} else {
		return 0;
	}
}
