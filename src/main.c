#include <stdio.h>
#include <syslog.h>
#include <glib.h>
#include <gmodule.h>
#include <time.h>

#include "spmfilter.h"

typedef void (*LoadEngine) (SETTINGS *settings, MAILCONN *mconn);

int main (int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;
	GKeyFile *keyfile;
	gsize modules_length = 0;
	SETTINGS *settings;
	MAILCONN *mconn;
	HEADER *header;
	clock_t start_process, stop_process;
	GModule *module;
	LoadEngine load_engine;
	gchar *engine_path;
	gchar **header_keys;
	gsize header_length = 0;
	int i;
	
	openlog("spmfilter", LOG_PID,LOG_MAIL);
	
	/* check queue dir */
	if (!g_file_test (QUEUE_DIR, G_FILE_TEST_EXISTS)) {
		syslog(LOG_INFO,"queue directory not available, will create it...");
		
		if(g_mkdir_with_parents(QUEUE_DIR,0700)!=0) {
			syslog(LOG_ERR,"Could not create queue dir!");
			return -1;
		}
		
	}
	
	mconn = g_slice_new(MAILCONN);

	settings = g_slice_new(SETTINGS);
	settings->debug = FALSE;
	
	/* all cmd args */
	GOptionEntry entries[] = {
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &settings->debug, "verbose logging", NULL},
		{ "file", 'f', 0, G_OPTION_ARG_STRING, &settings->config_file, "alternate config file", NULL},
		{ NULL }
	};
	
	/* parse cmd args */
	context = g_option_context_new ("- spmfilter options");
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print("%s\n", error->message);
		return 0;
	}
	
	
	/* read config */
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}
	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		printf("Error loading config: %s\n",error->message);
		return -1;
	}
	
	if (!settings->debug) 
		settings->debug = g_key_file_get_boolean(keyfile, "global", "debug", NULL);
	
	settings->engine = g_key_file_get_string(keyfile, "global", "engine", &error);
	if (settings->engine == NULL) {
		printf("%s\n", error->message);
		return -1;
	}
	
	mconn->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
	if (mconn->nexthop == NULL) {
		printf("%s\n", error->message);
		return -1;
	}
	
	settings->spool_dir = g_key_file_get_string(keyfile, "global", "spool_dir",NULL);
	if (settings->spool_dir == NULL) 
		settings->spool_dir = "/var/spool/spmfilter";
	
	settings->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
	if (settings->modules == NULL) {
		printf("%s\n",error->message);
		return -1;
	} else {
		if (settings->debug) {
			for(i = 0; settings->modules[i] != NULL; i++)
				syslog(LOG_DEBUG,"added module %s\n",settings->modules[i]);
		}
	} 
	
	header_keys = g_key_file_get_keys(keyfile,"header_checks",&header_length,NULL);
	mconn->header_checks = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
	
	while (header_length--) {
		header = g_slice_new (HEADER);
		header->name = g_key_file_get_string(keyfile, "header_checks", header_keys[header_length],NULL);

		g_hash_table_insert(
			mconn->header_checks,
			g_strdup(header_keys[header_length]),
			header);
		if (settings->debug) 
			syslog(LOG_DEBUG,
				"mconn->header_checks: added %s:%s",
				header_keys[header_length],
				header->name);
	}

	g_strfreev(header_keys);
	
	if (settings->debug) 
		start_process = clock();
	
	engine_path = g_module_build_path(LIB_DIR,settings->engine);
	module = g_module_open(engine_path, G_MODULE_BIND_LAZY);
	if (!module) {
		printf("%s\n", g_module_error ());
		return -1;
	}
	
	
	if (!g_module_symbol (module, "load", (gpointer *)&load_engine)) {
		printf("%s", g_module_error ());
		return -1;
	}
	
	load_engine(settings,mconn);
	
	if (settings->debug) {
		stop_process = clock();
		syslog(LOG_DEBUG,"processing time: %0.5f sec.", (float)(stop_process-start_process)/CLOCKS_PER_SEC);
	}
	
	if (!g_module_close (module))
		g_warning ("%s", g_module_error ());
		
	g_strfreev(settings->modules);
	g_slice_free(SETTINGS,settings);
	g_slist_free(mconn->rcpt);
	g_slice_free(MAILCONN,mconn);
	g_free(engine_path);
	return 0;
}
