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
#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gmodule.h>
#include <syslog.h>

#include "spmfilter_config.h"
#include "smf_settings.h"
#include "smf_settings_private.h"
#include "smf_trace.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_core.h"
#include "smf_platform.h"
#include "smf_daemon.h"
#include "smf_modules.h"

#define THIS_MODULE "spmfilter"

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;
	int ret;
	int debug = 0;
	char *config_file = NULL;
	SMFSettings_T *settings = NULL;

	/* all cmd args */
	GOptionEntry entries[] = {
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &debug, "verbose logging", NULL},
		{ "file", 'f', 0, G_OPTION_ARG_STRING, &config_file, "alternate config file", NULL},
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

	openlog("spmfilter", LOG_PID, LOG_MAIL);
	
//	g_thread_init(NULL);
	
//	if (!g_thread_supported()) {
//		g_print("glib2 does not support threads!");
//		return -1;
//	} else {
		settings = smf_settings_get();
//	} 
	
	/* parse config file and fill settings struct */
	if (smf_settings_parse_config(&settings,config_file) != 0)
		return -1;

	if (config_file != NULL)
		free(config_file);

	if (debug == 1)
		smf_settings_set_debug(debug);

	/* connect to database/ldap server, if necessary */
	if(settings->backend != NULL) {
		if (smf_lookup_connect() != 0) {
			TRACE(TRACE_ERR,"Unable to establish lookup connection!");
			return -1;
		}
	}

	/* check queue dir */
	if (!g_file_test (settings->queue_dir, G_FILE_TEST_EXISTS)) {
		TRACE(TRACE_INFO,"queue directory not available, will create it...");
		
		if(g_mkdir_with_parents(settings->queue_dir,0700)!=0) {
			TRACE(TRACE_ERR,"Could not create queue dir!");
			return -1;
		}
	}

	if (!g_module_supported()) {
		TRACE(TRACE_ERR,"glib2 does not support dynamic loading of modules!");
		return -1;
	}

	/* init gmime */
//	g_mime_init(0);
	
	if (settings->daemon == 1)
		ret = smf_daemon_mainloop(settings);
	else
		ret = smf_modules_engine_load(settings, dup(0));

	/* shutdown gmime */
//	g_mime_shutdown();

	if(settings->backend != NULL) {
		if (smf_lookup_disconnect() != 0)
			TRACE(TRACE_ERR,"Unable to destroy lookup connection!");
	}

	/* free all stuff */
	smf_settings_free(settings);

	if (ret != 0) {
		return -1;
	} else {
		return 0;
	}
}
