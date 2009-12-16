#include <stdio.h>
#include <glib.h>
#include <gmodule.h>
#include <time.h>
#include <stdlib.h>

#include "spmfilter.h"

#define THIS_MODULE "spmfilter"

typedef int (*LoadEngine) (MAILCONN *mconn);
GPrivate *settings_key = NULL;

int parse_config(void) {
	GError *error = NULL;
	GKeyFile *keyfile;
	gchar **code_keys;
	gsize modules_length = 0;
	gsize codes_length = 0;
	char *code_msg;
	int i, code;
	
	SETTINGS *settings = g_private_get(settings_key);
	
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}
	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		return -1;
	}
	
	settings->debug =  g_key_file_get_boolean(keyfile, "global", "debug", NULL);
	
	settings->queue_dir = g_key_file_get_string(keyfile, "global", "queue_dir", &error);
	if (settings->queue_dir == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		return -1;
	}
	
	settings->engine = g_key_file_get_string(keyfile, "global", "engine", &error);
	if (settings->engine == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		return -1;
	}
	
	settings->spool_dir = g_key_file_get_string(keyfile, "global", "spool_dir",NULL);
	if (settings->spool_dir == NULL) 
		settings->spool_dir = "/var/spool/spmfilter";
	
	settings->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
	if (settings->modules == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		return -1;
	} 
	
	settings->module_fail = g_key_file_get_integer(keyfile, "global", "module_fail",NULL);
	if (!settings->module_fail) {
		settings->module_fail = 3;
	}
	
	settings->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
	if (settings->nexthop == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		return -1;
	}

	settings->backend = g_key_file_get_string(keyfile, "global", "backend", NULL);

	TRACE(TRACE_DEBUG, "settings->engine: %s", settings->engine);
	TRACE(TRACE_DEBUG, "settings->spool_dir: %s", settings->spool_dir);
	for(i = 0; settings->modules[i] != NULL; i++) {
		TRACE(TRACE_DEBUG, "settings->modules: %s", settings->modules[i]);
	}
	TRACE(TRACE_DEBUG, "settings->module_fail: %d", settings->module_fail);
	TRACE(TRACE_DEBUG, "settings->nexthop: %s", settings->nexthop);
	TRACE(TRACE_DEBUG, "settings->backend: %s", settings->backend);

#ifdef HAVE_ZDB
	settings->sql_driver = g_key_file_get_string(keyfile, "sql", "driver", NULL);
	settings->sql_name = g_key_file_get_string(keyfile, "sql", "name", NULL);
	settings->sql_host = g_key_file_get_string(keyfile, "sql", "host", NULL);
	settings->sql_port = g_key_file_get_integer(keyfile, "sql", "port", NULL);
	settings->sql_user = g_key_file_get_string(keyfile, "sql", "user", NULL);
	settings->sql_pass = g_key_file_get_string(keyfile, "sql", "pass", NULL);
	settings->sql_user_query = g_key_file_get_string(keyfile, "sql", "user_query", NULL);
	settings->sql_encoding = g_key_file_get_string(keyfile, "sql", "encoding", NULL);
	settings->sql_max_connections = g_key_file_get_integer(keyfile, "sql", "max_connections", NULL);
	if (!settings->sql_max_connections)
		settings->sql_max_connections = 3;

	TRACE(TRACE_DEBUG, "settings->sql_driver: %s", settings->sql_driver);
	TRACE(TRACE_DEBUG, "settings->sql_name: %s", settings->sql_name);
	TRACE(TRACE_DEBUG, "settings->sql_host: %s", settings->sql_host);
	TRACE(TRACE_DEBUG, "settings->sql_port: %d", settings->sql_port);
	TRACE(TRACE_DEBUG, "settings->sql_user: %s", settings->sql_user);
	TRACE(TRACE_DEBUG, "settings->sql_pass: %s", settings->sql_pass);
	TRACE(TRACE_DEBUG, "settings->sql_user_query: %s", settings->sql_user_query);
	TRACE(TRACE_DEBUG, "settings->sql_encoding: %s", settings->sql_encoding);
	TRACE(TRACE_DEBUG, "settings->sql_max_connections: %d", settings->sql_max_connections);

	if(sql_connect() != 0) 
		return -1;
#endif 	

#ifdef HAVE_LDAP
	settings->ldap_uri = g_key_file_get_string(keyfile,"ldap","uri",NULL);
	settings->ldap_host = g_key_file_get_string(keyfile,"ldap","host",NULL);
	
	if (settings->ldap_uri == NULL & settings->ldap_host == NULL) {
		TRACE(TRACE_ERR, "config error: neither ldap uri nor ldap host supplied");
		return -1;
	}
	
	settings->ldap_port = g_key_file_get_integer(keyfile,"ldap","port",NULL);
	if (!settings->ldap_port)
		settings->ldap_port = 389;
	settings->ldap_binddn = g_key_file_get_string(keyfile,"ldap","binddn",NULL);
	settings->ldap_bindpw = g_key_file_get_string(keyfile,"ldap","bindpw",NULL);

	settings->ldap_base = g_key_file_get_string(keyfile,"ldap","base",&error);
	if (settings->ldap_base == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		return -1;
	}
	
	settings->ldap_referrals = g_key_file_get_boolean(keyfile, "ldap","referrals",NULL);
	
	TRACE(TRACE_DEBUG, "settings->ldap_uri: %s", settings->ldap_uri);
	TRACE(TRACE_DEBUG, "settings->ldap_host: %s", settings->ldap_host);
	TRACE(TRACE_DEBUG, "settings->ldap_port: %d", settings->ldap_port);
	TRACE(TRACE_DEBUG, "settings->ldap_binddn: %s", settings->ldap_binddn);
	TRACE(TRACE_DEBUG, "settings->ldap_bindpw: %s", settings->ldap_bindpw);
	TRACE(TRACE_DEBUG, "settings->ldap_base: %s", settings->ldap_base);
	TRACE(TRACE_DEBUG, "settings->ldap_referrals: %d", settings->ldap_referrals);
	
	if(ldap_connect() != 0) 
		return -1;
	
#endif
	
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
	
	TRACE(TRACE_DEBUG, "settings->nexthop_fail_code: %d", settings->nexthop_fail_code);
	TRACE(TRACE_DEBUG, "settings->nexthop_fail_msg: %s", settings->nexthop_fail_msg);
	
	code_keys = g_key_file_get_keys(keyfile,"smtpd",&codes_length,NULL);
	settings->smtp_codes = g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
	while (codes_length--) {
		/* only insert smtp codes to hashtable */
		code =g_ascii_strtod(code_keys[codes_length],NULL);
		if ((code > 400) & (code < 600)) {
			code_msg = g_key_file_get_string(keyfile, "smtpd", code_keys[codes_length],NULL);
			g_hash_table_insert(
				settings->smtp_codes,
				g_strdup(code_keys[codes_length]),
				code_msg
				);
			TRACE(TRACE_DEBUG,
				"settings->smtp_codes: append %s=%s",
				code_keys[codes_length],code_msg);
			free(code_msg);
		}
	}
	g_strfreev(code_keys);
	g_private_set(settings_key, settings);
	
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
	
	g_mime_init(0);
	
	mconn = g_slice_new(MAILCONN);
	settings = g_slice_new(SETTINGS);
	settings->debug = 0;
	
	if (!g_thread_supported()) g_thread_init(NULL);
	if (!settings_key) settings_key = g_private_new(g_free);
	
	/* all cmd args */
	GOptionEntry entries[] = {
		{ "debug", 'd', 0, G_OPTION_ARG_NONE, &settings->debug, "verbose logging", NULL},
		{ "file", 'f', 0, G_OPTION_ARG_STRING, &settings->config_file, "alternate config file", NULL},
		{ NULL }
	};
	
	g_private_set(settings_key, settings);
	
	/* parse cmd args */
	context = g_option_context_new ("- spmfilter options");
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error)) {
		g_print("%s\n", error->message);
		g_error_free(error);
		return 0;
	}
	
	if (parse_config()!=0) 
		return -1;

	/* check queue dir */
	if (!g_file_test (settings->queue_dir, G_FILE_TEST_EXISTS)) {
		TRACE(TRACE_INFO,"queue directory not available, will create it...");
		
		if(g_mkdir_with_parents(settings->queue_dir,0700)!=0) {
			TRACE(TRACE_ERR,"Could not create queue dir!");
			return -1;
		}
	}

	start_process = clock();
	
	/* check if engine module starts with lib */
	if (g_str_has_prefix(settings->engine,"lib")) {
		engine_path = g_module_build_path(LIB_DIR,settings->engine);
	} else {
		engine_path = g_module_build_path(LIB_DIR,g_strdup_printf("lib%s",settings->engine));
	}
	module = g_module_open(engine_path, G_MODULE_BIND_LAZY);
	if (!module) {
		TRACE(TRACE_ERR,"%s\n", g_module_error());
		return -1;
	}
	
	if (!g_module_symbol(module, "load", (gpointer *)&load_engine)) {
		TRACE(TRACE_ERR,"%s", g_module_error());
		return -1;
	}

	ret = load_engine(mconn);

	stop_process = clock();
	if (settings->debug) {
		TRACE(TRACE_DEBUG,"processing time: %0.5f sec.", (float)(stop_process-start_process)/CLOCKS_PER_SEC);
	}
	
	if (!g_module_close(module))
		TRACE(TRACE_WARNING,"%s", g_module_error());

	g_strfreev(settings->modules);
	g_slice_free(SETTINGS,settings);
	g_private_set(settings_key, NULL);
	g_error_free(error);
	g_slice_free(MAILCONN,mconn);
	g_free(engine_path);
	g_mime_shutdown();
	if (ret != 0) {
		return -1;
	} else {
		return 0;
	}
}
