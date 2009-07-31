#include <stdio.h>
#include <syslog.h>
#include <glib.h>
#include <gmodule.h>
#include <time.h>

#include "spmfilter.h"

typedef int (*LoadEngine) (SETTINGS *settings, MAILCONN *mconn);

int parse_config(SETTINGS *settings) {
	GError *error = NULL;
	GKeyFile *keyfile;
	HEADER *header;
	gchar **header_keys, **code_keys;
	gsize modules_length = 0;
	gsize header_length = 0;
	gsize codes_length = 0;
	char *code_msg;
	int i;

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
	
	settings->spool_dir = g_key_file_get_string(keyfile, "global", "spool_dir",NULL);
	if (settings->spool_dir == NULL) 
		settings->spool_dir = "/var/spool/spmfilter";
	
	settings->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
	if (settings->modules == NULL) {
		printf("%s\n",error->message);
		return -1;
	} 
	
	settings->module_fail = g_key_file_get_integer(keyfile, "global", "module_fail",NULL);
	if (!settings->module_fail) {
		settings->module_fail = 3;
	}
	
	settings->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
	if (settings->nexthop == NULL) {
		printf("%s\n", error->message);
		return -1;
	}

	if (settings->debug) {
		syslog(LOG_DEBUG, "settings->debug: %d", settings->debug);
		syslog(LOG_DEBUG, "settings->engine: %s", settings->engine);
		syslog(LOG_DEBUG, "settings->spool_dir: %s", settings->spool_dir);
		for(i = 0; settings->modules[i] != NULL; i++) {
			syslog(LOG_DEBUG, "settings->modules: %s", settings->modules[i]);
		}
		syslog(LOG_DEBUG, "settings->module_fail: %d", settings->module_fail);
		syslog(LOG_DEBUG, "settings->nexthop: %s", settings->nexthop);
	}

	/* header_checks group */
	header_keys = g_key_file_get_keys(keyfile,"header_checks",&header_length,NULL);
	settings->header_checks = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
	
	while (header_length--) {
		header = g_slice_new (HEADER);
		header->name = g_key_file_get_string(keyfile, "header_checks", header_keys[header_length],NULL);

		g_hash_table_insert(
			settings->header_checks,
			g_strdup(header_keys[header_length]),
			header);
		if (settings->debug) 
			syslog(LOG_DEBUG,
				"settings->header_checks: append %s=%s",
				header_keys[header_length],
				header->name);
	}
	g_strfreev(header_keys);
	
	/* smtpd group */
	settings->nexthop_fail_code = g_key_file_get_integer(keyfile, "smtpd", "nexthop_fail_code", NULL);
	if (!settings->nexthop_fail_code) {
		/* nexthop_fail_code not configured, now we use default vaules */
		settings->nexthop_fail_code = 451;
	}
	
	settings->nexthop_fail_msg = g_key_file_get_string(keyfile, "smtpd", "nexthop_fail_msg", NULL);
	if (settings->nexthop_fail_msg == NULL) {
		/* nexthop_fail_msg not configured, now we use default vaules */
		settings->nexthop_fail_msg = "Requested action aborted: local error in processing";
	}
	
	if (settings->debug) {
		syslog(LOG_DEBUG, "settings->nexthop_fail_code: %d", settings->nexthop_fail_code);
		syslog(LOG_DEBUG, "settings->nexthop_fail_msg: %s", settings->nexthop_fail_msg);
	}
	
	code_keys = g_key_file_get_keys(keyfile,"smtpd",&codes_length,NULL);
	settings->smtp_codes = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
	while (codes_length--) {
		/* only insert smtp codes to hashtable */
		if (g_regex_match_simple("^\\d{3}$",code_keys[codes_length],0,0)) {
			code_msg = g_key_file_get_string(keyfile, "smtpd", code_keys[codes_length],NULL);
			g_hash_table_insert(
				settings->smtp_codes,
				g_strdup(code_keys[codes_length]),
				code_msg
				);
			if (settings->debug)
				syslog(LOG_DEBUG,
				"settings->smtp_codes: append %s=%s",
				code_keys[codes_length],code_msg);
			free(code_msg);
		}
	}
	g_strfreev(code_keys);
	return 0;
}

int main(int argc, char *argv[]) {
	GError *error = NULL;
	GOptionContext *context;
	SETTINGS *settings;
	MAILCONN *mconn;
	clock_t start_process, stop_process;
	GModule *module;
	LoadEngine load_engine;
	gchar *engine_path;
	int ret;
	
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
	
	if (parse_config(settings)!=0) 
		return -1;
	
	/* initialize runtime_data hashtable */
	mconn->runtime_data = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
	
	if (settings->debug) 
		start_process = clock();
	
	/* check if engine module starts with lib */
	if (g_str_has_prefix(settings->engine,"lib")) {
		engine_path = g_module_build_path(LIB_DIR,settings->engine);
	} else {
		engine_path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",settings->engine));
	}
	module = g_module_open(engine_path, G_MODULE_BIND_LAZY);
	if (!module) {
		printf("%s\n", g_module_error ());
		return -1;
	}
	
	if (!g_module_symbol(module, "load", (gpointer *)&load_engine)) {
		printf("%s", g_module_error ());
		return -1;
	}

	ret = load_engine(settings,mconn);
	
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
	
	if (ret != 0) {
		return -1;
	} else {
		return 0;
	}
}
