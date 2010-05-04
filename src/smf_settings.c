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

#include <glib.h>
#include <math.h>

#include "spmfilter_config.h"
#include "smf_smtp_codes.h"
#include "smf_settings.h"
#include "smf_trace.h"

#define THIS_MODULE "settings"

SMFSettings_T *settings = NULL;

SMFSettings_T *smf_settings_get(void) {
	if (settings == NULL) {
		settings = g_slice_new(SMFSettings_T);
		settings->debug = 0;
	}
	return settings;
}

void smf_settings_free(SMFSettings_T *settings) {
	smf_smtp_codes_free();
	g_strfreev(settings->modules);
	g_free(settings->config_file);
	g_free(settings->queue_dir);
	g_free(settings->engine);
	g_free(settings->nexthop);
	g_free(settings->nexthop_fail_msg);
	g_strfreev(settings->backend);
	g_slice_free(SMFSettings_T,settings);
}

int smf_settings_parse_config(void) {
	GError *error = NULL;
	GKeyFile *keyfile;
	gchar **code_keys;
	gsize modules_length = 0;
	gsize backend_length = 0;
	gsize codes_length = 0;
	gsize sql_num_hosts = 0;
	gsize ldap_num_hosts = 0;
	char *code_msg;
	int i, code;
	SMFSettings_T *settings = smf_settings_get();

	/* fallback to default config path,
	 * if config file is not defined as
	 * command argument */
	if (settings->config_file == NULL) {
		settings->config_file = g_strdup("/etc/spmfilter.conf");
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
	} else 
		settings->engine = g_strstrip(settings->engine);
	
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

	settings->backend = g_key_file_get_string_list(keyfile, "global", "backend", &backend_length,NULL);
	if (backend_length == 0)
		settings->backend = NULL;
	settings->backend_connection = g_key_file_get_string(keyfile,"global","backend_connection",NULL);
	if (settings->backend_connection == NULL)
		settings->backend_connection = g_strdup("failover");
	else {
		settings->backend_connection = g_strstrip(settings->backend_connection);
		if ((g_ascii_strcasecmp(settings->backend_connection,"balance") != 0) &&
			(g_ascii_strcasecmp(settings->backend_connection,"failover") != 0)) {
			TRACE(TRACE_ERR,"invalid backend_connection option");
			return -1;
		}
	}

	TRACE(TRACE_DEBUG, "settings->engine: %s", settings->engine);
	for(i = 0; settings->modules[i] != NULL; i++) {
		TRACE(TRACE_DEBUG, "settings->modules: %s", settings->modules[i]);
	}
	TRACE(TRACE_DEBUG, "settings->module_fail: %d", settings->module_fail);
	TRACE(TRACE_DEBUG, "settings->nexthop: %s", settings->nexthop);
	if(settings->backend != NULL) {
		for (i=0; settings->backend[i] != NULL; i++) {
			TRACE(TRACE_DEBUG,"settings->backend: %s",settings->backend[i]);
		}
	}
	TRACE(TRACE_DEBUG, "settings->backend_connection: %s", settings->backend_connection);

	settings->add_header = g_key_file_get_boolean(keyfile, "global", "add_header",&error);
	if (settings->add_header == 0) {
		if (error != NULL) {
			if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
				settings->add_header = 1;
			}
			g_error_free(error);
			error = NULL;
		} 
	}
	TRACE(TRACE_DEBUG, "settings->add_header: %d", settings->add_header);

	settings->max_size = (unsigned long) floor(g_key_file_get_double(keyfile, "global", "max_size",NULL) + 0.5);
	if (!settings->max_size) {
		settings->max_size = 0;
	}
	TRACE(TRACE_DEBUG, "settings->max_size: %d", settings->max_size);

	settings->tls = g_key_file_get_integer(keyfile,"global","tls_enable",NULL);
	if (!settings->tls)
		settings->tls = 0;
	TRACE(TRACE_DEBUG, "settings->tls: %d", settings->tls);

	settings->tls_pass = g_key_file_get_string(keyfile,"global","tls_pass",NULL);
	TRACE(TRACE_DEBUG, "settings->tls_pass: %s", settings->tls_pass);

	settings->daemon = g_key_file_get_boolean(keyfile,"global","daemon",NULL);
	if (g_ascii_strcasecmp(settings->engine,"pipe") == 0) {
		TRACE(TRACE_ERR,"pipe engine can not be used in daemon mode");
		return -1;
	}
	TRACE(TRACE_DEBUG, "settings->daemon: %d", settings->daemon);

	settings->sql_driver = g_key_file_get_string(keyfile, "sql", "driver", NULL);
	settings->sql_name = g_key_file_get_string(keyfile, "sql", "name", &error);
	if(settings->backend != NULL) {
		for (i=0; settings->backend[i] != NULL; i++) {
			if (g_ascii_strcasecmp(settings->backend[i],"sql") == 0) {
				if (settings->sql_name == NULL) {
					TRACE(TRACE_ERR, "config error: %s", error->message);
					g_error_free(error);
					return -1;
				}
				break;
			}
		}
	}
	settings->sql_host = g_key_file_get_string_list(keyfile, "sql", "host", &sql_num_hosts,NULL);
	settings->sql_num_hosts = sql_num_hosts;
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
	if (settings->sql_host != NULL) {
		for(i = 0; settings->sql_host[i] != NULL; i++) {
			TRACE(TRACE_DEBUG, "settings->sql_host: %s", settings->sql_host[i]);
		}
	}
	TRACE(TRACE_DEBUG, "settings->sql_port: %d", settings->sql_port);
	TRACE(TRACE_DEBUG, "settings->sql_user: %s", settings->sql_user);
	TRACE(TRACE_DEBUG, "settings->sql_pass: %s", settings->sql_pass);
	TRACE(TRACE_DEBUG, "settings->sql_user_query: %s", settings->sql_user_query);
	TRACE(TRACE_DEBUG, "settings->sql_encoding: %s", settings->sql_encoding);
	TRACE(TRACE_DEBUG, "settings->sql_max_connections: %d", settings->sql_max_connections);

	settings->ldap_uri = g_key_file_get_string(keyfile,"ldap","uri",NULL);
	settings->ldap_host = g_key_file_get_string_list(keyfile, "ldap", "host", &ldap_num_hosts,NULL);
	settings->ldap_num_hosts = ldap_num_hosts;

	if(settings->backend != NULL) {
		for (i=0; settings->backend[i] != NULL; i++) {
			if (g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) {
				if (settings->ldap_uri == NULL && settings->ldap_host == NULL) {
					TRACE(TRACE_ERR, "config error: neither ldap uri nor ldap host supplied");
					return -1;
				}
				break;
			}
		}
	}
	settings->ldap_port = g_key_file_get_integer(keyfile,"ldap","port",NULL);
	if (!settings->ldap_port)
		settings->ldap_port = 389;
	settings->ldap_binddn = g_key_file_get_string(keyfile,"ldap","binddn",NULL);
	settings->ldap_bindpw = g_key_file_get_string(keyfile,"ldap","bindpw",NULL);

	settings->ldap_base = g_key_file_get_string(keyfile,"ldap","base",&error);
	if(settings->backend != NULL) {
		for (i=0; settings->backend[i] != NULL; i++) {
			if ((g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) && (settings->ldap_base == NULL)) {
				TRACE(TRACE_ERR, "config error: %s", error->message);
				g_error_free(error);
				return -1;
			}
		}
	}

	settings->ldap_referrals = g_key_file_get_boolean(keyfile, "ldap","referrals",NULL);

	settings->ldap_scope = g_key_file_get_string(keyfile, "ldap", "scope", NULL);
	if (settings->ldap_scope != NULL && settings->backend != NULL) {
		for (i=0; settings->backend[i] != NULL; i++) {
			if (g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) {
				if ((g_ascii_strcasecmp(settings->ldap_scope,"subtree") != 0) &&
						(g_ascii_strcasecmp(settings->ldap_scope,"onelevel") != 0) &&
						(g_ascii_strcasecmp(settings->ldap_scope,"base") != 0)) {
					TRACE(TRACE_ERR, "invalid ldap scope");
					return -1;
				}
			}
		}
	} else {
		settings->ldap_scope = g_strdup("subtree");
	}
	
	settings->ldap_user_query = g_key_file_get_string(keyfile, "ldap", "user_query", NULL);

	TRACE(TRACE_DEBUG, "settings->ldap_uri: %s", settings->ldap_uri);
	if (settings->ldap_host != NULL) {
		for(i = 0; settings->ldap_host[i] != NULL; i++) {
			TRACE(TRACE_DEBUG, "settings->ldap_host: %s", settings->ldap_host[i]);
		}
	}
	TRACE(TRACE_DEBUG, "settings->ldap_port: %d", settings->ldap_port);
	TRACE(TRACE_DEBUG, "settings->ldap_binddn: %s", settings->ldap_binddn);
	TRACE(TRACE_DEBUG, "settings->ldap_bindpw: %s", settings->ldap_bindpw);
	TRACE(TRACE_DEBUG, "settings->ldap_base: %s", settings->ldap_base);
	TRACE(TRACE_DEBUG, "settings->ldap_referrals: %d", settings->ldap_referrals);
	TRACE(TRACE_DEBUG, "settings->ldap_scope: %s", settings->ldap_scope);
	TRACE(TRACE_DEBUG, "settings->ldap_user_query: %s", settings->ldap_user_query);

	/* smtpd group */
	settings->nexthop_fail_code = g_key_file_get_integer(keyfile, "smtpd", "nexthop_fail_code", NULL);
	if (!settings->nexthop_fail_code) {
		/* nexthop_fail_code not configured, now we use default vaules */
		settings->nexthop_fail_code = 451;
	}

	settings->nexthop_fail_msg = g_key_file_get_string(keyfile, "smtpd", "nexthop_fail_msg", NULL);
	if (settings->nexthop_fail_msg == NULL) {
		/* nexthop_fail_msg not configured, now we use default vaules */
		settings->nexthop_fail_msg = g_strdup("Requested action aborted: local error in processing");
	}

	TRACE(TRACE_DEBUG, "settings->nexthop_fail_code: %d", settings->nexthop_fail_code);
	TRACE(TRACE_DEBUG, "settings->nexthop_fail_msg: %s", settings->nexthop_fail_msg);

	code_keys = g_key_file_get_keys(keyfile,"smtpd",&codes_length,NULL);
	while (codes_length--) {
		/* only insert smtp codes to hashtable */
		code = g_ascii_strtod(code_keys[codes_length],NULL);
		if ((code > 400) && (code < 600)) {
			code_msg = g_key_file_get_string(keyfile, "smtpd", code_keys[codes_length],NULL);
			smf_smtp_codes_insert(code,code_msg);
			TRACE(TRACE_DEBUG,"settings->smtp_codes: append %d=%s",code,smf_smtp_codes_get(code));
			g_free(code_msg);
		}
	}
	g_strfreev(code_keys);
	g_key_file_free(keyfile);
	
	return 0;
}
