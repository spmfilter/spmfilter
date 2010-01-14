#include <glib.h>
#include "spmfilter.h"

#define THIS_MODULE "settings"

SETTINGS *settings = NULL;

SETTINGS *get_settings(void) {
	if (settings == NULL) {
		settings = g_slice_new(SETTINGS);
		settings->debug = 0;
	}
	return settings;
}

void set_settings(SETTINGS **s) {
	settings = *s;
}

void free_settings(SETTINGS *settings) {
//	smtp_codes_free(&settings->smtp_codes);
	g_strfreev(settings->modules);
	g_free(settings->config_file);
	g_free(settings->queue_dir);
	g_free(settings->engine);
	g_free(settings->nexthop);
	g_free(settings->nexthop_fail_msg);
	g_free(settings->backend);

	g_slice_free(SETTINGS,settings);
}

int parse_config(void) {
	GError *error = NULL;
	GKeyFile *keyfile;
	gchar **code_keys;
	gsize modules_length = 0;
	gsize codes_length = 0;
	char *code_msg;
	int i, code;
	SETTINGS *settings = get_settings();

	/* fallback to default config path,
	 * if config file is not defined as
	 * command argument */
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}

	/* open config file and start parsing */
	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		return -1;
	}

	/* parse general settings */
	settings->debug =  g_key_file_get_boolean(keyfile, "global", "debug", NULL);

	settings->queue_dir = g_key_file_get_string(keyfile, "global", "queue_dir", NULL);
	if (settings->queue_dir == NULL)
		settings->queue_dir = g_strdup("/var/spool/spmfilter");

	settings->engine = g_key_file_get_string(keyfile, "global", "engine", &error);
	if (settings->engine == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	settings->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
	if (settings->modules == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	settings->module_fail = g_key_file_get_integer(keyfile, "global", "module_fail",NULL);
	if (!settings->module_fail) {
		settings->module_fail = 3;
	}

	settings->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
	if (settings->nexthop == NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	settings->backend = g_key_file_get_string(keyfile, "global", "backend", NULL);


	TRACE(TRACE_DEBUG, "settings->engine: %s", settings->engine);
	for(i = 0; settings->modules[i] != NULL; i++) {
		TRACE(TRACE_DEBUG, "settings->modules: %s", settings->modules[i]);
	}
	TRACE(TRACE_DEBUG, "settings->module_fail: %d", settings->module_fail);
	TRACE(TRACE_DEBUG, "settings->nexthop: %s", settings->nexthop);
	TRACE(TRACE_DEBUG, "settings->backend: %s", settings->backend);

#ifdef HAVE_ZDB
	/* if spmfilter is compiled with zdb,
	 * we also need the sql group */
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
	/* if spmfilter is compiled with ldap,
	 * we also need the ldap group */
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
		g_error_free(error);
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
	while (codes_length--) {
		/* only insert smtp codes to hashtable */
		code = g_ascii_strtod(code_keys[codes_length],NULL);
		if ((code > 400) & (code < 600)) {
			code_msg = g_key_file_get_string(keyfile, "smtpd", code_keys[codes_length],NULL);
			// TODO: append_code missing
			g_free(code_msg);
	//		TRACE(TRACE_DEBUG,"settings->smtp_codes: append %s=%s",settings->smtp_codes->code,settings->smtp_codes->message);
		}
	}
	g_strfreev(code_keys);
	g_key_file_free(keyfile);
	set_settings(&settings);
	return 0;
}
