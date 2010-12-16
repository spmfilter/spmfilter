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

#include <string.h>
#include <glib.h>

#include "spmfilter_config.h"
#include "smf_smtp_codes.h"
#include "smf_settings.h"
#include "smf_trace.h"

#define THIS_MODULE "settings"

static GMutex *settings_mutex = NULL;

SMFSettings_T *smf_settings_get(void) {
	static SMFSettings_T *settings = NULL;
	
	if (settings == NULL) {
		settings_mutex = g_mutex_new();
		g_mutex_lock(settings_mutex);
		settings = g_slice_new(SMFSettings_T);
		settings->debug = 0;
		settings->config_file = NULL;
		settings->queue_dir = NULL;
		settings->engine = NULL;
		settings->modules = NULL;
		settings->nexthop = NULL;
		settings->nexthop_fail_msg = NULL;
		settings->backend = NULL;
		settings->backend_connection = NULL;
		settings->tls_pass = NULL;
		settings->sql_driver = NULL;
		settings->sql_name = NULL;
		settings->sql_host = NULL;
		settings->sql_user = NULL;
		settings->sql_pass = NULL;
		settings->sql_user_query = NULL;
		settings->sql_encoding = NULL;
		settings->ldap_uri = NULL;
		settings->ldap_host = NULL;
		settings->ldap_binddn = NULL;
		settings->ldap_bindpw = NULL;
		settings->ldap_base = NULL;
		settings->ldap_scope = NULL;
		settings->ldap_user_query = NULL;
		settings->module_fail = 3;
		settings->nexthop_fail_code = 451;
		settings->add_header = 1;
		settings->max_size = 0;
		settings->tls = 0;
		settings->daemon = 0;
		settings->sql_max_connections = 3;
		settings->sql_port = 0;
		g_mutex_unlock(settings_mutex);
	}
	return settings;
}

void smf_settings_free(SMFSettings_T *settings) {
	g_mutex_lock(settings_mutex);
	smf_smtp_codes_free();
	g_strfreev(settings->modules);
	g_strfreev(settings->backend);
	g_free(settings->config_file);
	g_free(settings->queue_dir);
	g_free(settings->engine);
	g_free(settings->nexthop);
	g_free(settings->nexthop_fail_msg);
	g_free(settings->backend_connection);
	g_free(settings->tls_pass);
	g_free(settings->sql_driver);
	g_free(settings->sql_name);
	g_strfreev(settings->sql_host);
	g_free(settings->sql_user);
	g_free(settings->sql_pass);
	g_free(settings->sql_user_query);
	g_free(settings->sql_encoding);
	g_free(settings->ldap_uri);
	g_strfreev(settings->ldap_host);
	g_free(settings->ldap_binddn);
	g_free(settings->ldap_bindpw);
	g_free(settings->ldap_base);
	g_free(settings->ldap_scope);
	g_free(settings->ldap_user_query);

	g_slice_free(SMFSettings_T,settings);
	g_mutex_unlock(settings_mutex);
	g_mutex_free(settings_mutex);
}

int smf_settings_parse_config(SMFSettings_T *settings, char *alternate_file) {
	GError *error = NULL;
	GKeyFile *keyfile;
	gchar **code_keys;
	gsize modules_length = 0;
	gsize backend_length = 0;
	gsize codes_length = 0;
	gsize sql_num_hosts = 0;
	gsize ldap_num_hosts = 0;
	char *code_msg = NULL;
	int i, code;

	g_mutex_lock(settings_mutex);
	/* fallback to default config path,
	 * if config file is not defined as
	 * command argument */
	if (alternate_file != NULL) {
		settings->config_file = g_strdup(alternate_file);
	} else {
		settings->config_file = g_strdup("/etc/spmfilter.conf");
	}

	/* open config file and start parsing */
	keyfile = g_key_file_new();
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
	if (error != NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	} else 
		settings->engine = g_strstrip(settings->engine);
		
	TRACE(TRACE_DEBUG, "settings->engine: %s", settings->engine);
	
	settings->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
	if (error != NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	settings->module_fail = g_key_file_get_integer(keyfile, "global", "module_fail",NULL);

	settings->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
	if (error != NULL) {
		TRACE(TRACE_ERR, "config error: %s", error->message);
		g_error_free(error);
		return -1;
	}

	settings->backend = g_key_file_get_string_list(keyfile, "global", "backend", &backend_length,NULL);
	settings->backend_connection = g_key_file_get_string(keyfile,"global","backend_connection",NULL);
	if (settings->backend_connection == NULL)
		settings->backend_connection = g_strdup("failover");
	else {
		if (strlen(settings->backend_connection) > 0) {
			settings->backend_connection = g_strstrip(settings->backend_connection);
			if ((g_ascii_strcasecmp(settings->backend_connection,"balance") != 0) &&
					(g_ascii_strcasecmp(settings->backend_connection,"failover") != 0)) {
				TRACE(TRACE_ERR,"invalid backend_connection option");
				return -1;
			}
		}
	}

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
	
	if (g_key_file_get_boolean(keyfile, "global", "add_header",&error)) {
		settings->add_header = 1;
	} else {
		if (error != NULL) {
			if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
				settings->add_header = 1;
			}
			g_error_free(error);
		} else {
			settings->add_header = 0;
		}
	}
	TRACE(TRACE_DEBUG, "settings->add_header: %d", settings->add_header);

	settings->max_size = g_key_file_get_uint64(keyfile, "global", "max_size",NULL);
	TRACE(TRACE_DEBUG, "settings->max_size: %d", settings->max_size);

	settings->tls = g_key_file_get_integer(keyfile,"global","tls_enable",NULL);
	TRACE(TRACE_DEBUG, "settings->tls: %d", settings->tls);

	settings->tls_pass = g_key_file_get_string(keyfile,"global","tls_pass",NULL);
	TRACE(TRACE_DEBUG, "settings->tls_pass: %s", settings->tls_pass);

	settings->daemon = g_key_file_get_boolean(keyfile,"global","daemon",NULL);
	if (g_ascii_strcasecmp(settings->engine,"pipe") == 0) {
		TRACE(TRACE_ERR,"pipe engine can not be used in daemon mode");
		return -1;
	} 
	TRACE(TRACE_DEBUG, "settings->daemon: %d", settings->daemon);

	if (g_key_file_has_group(keyfile,"sql")) {
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
		settings->sql_user_query = g_key_file_get_string(keyfile, "sql", "user_query", &error);
		if (settings->sql_user_query == NULL) {
			TRACE(TRACE_ERR, "config error: %s", error->message);
			g_error_free(error);
			return -1;
		}

		settings->sql_encoding = g_key_file_get_string(keyfile, "sql", "encoding", NULL);
		settings->sql_max_connections = g_key_file_get_integer(keyfile, "sql", "max_connections", NULL);

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
	} else {
		if(settings->backend != NULL) {
			for (i=0; settings->backend[i] != NULL; i++) {
				if (g_ascii_strcasecmp(settings->backend[i],"sql") == 0) {
					TRACE(TRACE_ERR,"Can't find settings group sql");
					return -1;
				}
			}
		}
	}

	if (g_key_file_has_group(keyfile,"ldap")) {
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

		settings->ldap_user_query = g_key_file_get_string(keyfile, "ldap", "user_query", &error);
		if (settings->ldap_user_query == NULL) {
			TRACE(TRACE_ERR, "config error: %s", error->message);
			g_error_free(error);
			return -1;
		}

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
	} else {
		if(settings->backend != NULL) {
			for (i=0; settings->backend[i] != NULL; i++) {
				if (g_ascii_strcasecmp(settings->backend[i],"ldap") == 0) {
					TRACE(TRACE_ERR,"Can't find settings group ldap");
					return -1;
				}
			}
		}
	}
	
	/* smtpd group */
	settings->nexthop_fail_code = g_key_file_get_integer(keyfile, "smtpd", "nexthop_fail_code", NULL);

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
	g_mutex_unlock (settings_mutex);
	
	return 0;
}
