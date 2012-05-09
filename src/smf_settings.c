/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
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

#define THIS_MODULE "settings"

#include "smf_settings.h"
#include "smf_settings_private.h"

/* has to be removed */
SMFSettings_T *smf_settings_new(void) {
    SMFSettings_T *settings = NULL;

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
    settings->smtp_codes = g_hash_table_new_full(g_str_hash, g_str_equal,free,free);
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

    return settings;
}

void smf_settings_free(SMFSettings_T *settings) {
    assert(settings);

    g_strfreev(settings->modules);
    g_strfreev(settings->backend);
    g_free(settings->config_file);
    g_free(settings->queue_dir);
    g_free(settings->engine);
    g_free(settings->nexthop);
    g_free(settings->nexthop_fail_msg);
    g_free(settings->backend_connection);
    g_free(settings->tls_pass);
    g_hash_table_destroy(settings->smtp_codes);
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
}

int smf_settings_parse_config(SMFSettings_T **settings, char *alternate_file) {
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

    assert(*settings);
    assert(alternate_file);

    /* fallback to default config path,
     * if config file is not defined as
     * command argument */
    if (alternate_file != NULL) {
        (*settings)->config_file = g_strdup(alternate_file);
    } else {
        (*settings)->config_file = g_strdup("/etc/spmfilter.conf");
    }

    /* open config file and start parsing */
    keyfile = g_key_file_new();
    if (!g_key_file_load_from_file (keyfile, (*settings)->config_file, G_KEY_FILE_NONE, &error)) {
        TRACE(TRACE_ERR,"Error loading config: %s",error->message);
        g_error_free(error);
        return -1;
    }

    /* parse general settings */
    (*settings)->debug =  g_key_file_get_boolean(keyfile, "global", "debug", NULL);

    (*settings)->queue_dir = g_key_file_get_string(keyfile, "global", "queue_dir", NULL);
    if ((*settings)->queue_dir == NULL)
        (*settings)->queue_dir = g_strdup("/var/spool/spmfilter");

    (*settings)->engine = g_key_file_get_string(keyfile, "global", "engine", &error);
    if (error != NULL) {
        TRACE(TRACE_ERR, "config error: %s", error->message);
        g_error_free(error);
        return -1;
    } else 
        (*settings)->engine = g_strstrip((*settings)->engine);
        
    TRACE(TRACE_DEBUG, "settings->engine: %s", (*settings)->engine);
    
    (*settings)->modules = g_key_file_get_string_list(keyfile,"global","modules",&modules_length,&error);
    if (error != NULL) {
        TRACE(TRACE_ERR, "config error: %s", error->message);
        g_error_free(error);
        return -1;
    }

    (*settings)->module_fail = g_key_file_get_integer(keyfile, "global", "module_fail",NULL);

    (*settings)->nexthop = g_key_file_get_string(keyfile, "global", "nexthop", &error);
    if (error != NULL) {
        TRACE(TRACE_ERR, "config error: %s", error->message);
        g_error_free(error);
        return -1;
    }

    (*settings)->backend = g_key_file_get_string_list(keyfile, "global", "backend", &backend_length,NULL);
    (*settings)->backend_connection = g_key_file_get_string(keyfile,"global","backend_connection",NULL);
    if ((*settings)->backend_connection == NULL)
        (*settings)->backend_connection = g_strdup("failover");
    else {
        if (strlen((*settings)->backend_connection) > 0) {
            (*settings)->backend_connection = g_strstrip((*settings)->backend_connection);
            if ((g_ascii_strcasecmp((*settings)->backend_connection,"balance") != 0) &&
                    (g_ascii_strcasecmp((*settings)->backend_connection,"failover") != 0)) {
                TRACE(TRACE_ERR,"invalid backend_connection option");
                return -1;
            }
        }
    }

    for(i = 0; (*settings)->modules[i] != NULL; i++) {
        TRACE(TRACE_DEBUG, "settings->modules: %s", (*settings)->modules[i]);
    }
    TRACE(TRACE_DEBUG, "settings->module_fail: %d", (*settings)->module_fail);
    TRACE(TRACE_DEBUG, "settings->nexthop: %s", (*settings)->nexthop);
    if((*settings)->backend != NULL) {
        for (i=0; (*settings)->backend[i] != NULL; i++) {
            TRACE(TRACE_DEBUG,"settings->backend: %s",(*settings)->backend[i]);
        }
    }
    TRACE(TRACE_DEBUG, "settings->backend_connection: %s", (*settings)->backend_connection);
    
    if (g_key_file_get_boolean(keyfile, "global", "add_header",&error)) {
        (*settings)->add_header = 1;
    } else {
        if (error != NULL) {
            if (error->code == G_KEY_FILE_ERROR_KEY_NOT_FOUND) {
                (*settings)->add_header = 1;
            }
            g_error_free(error);
        } else {
            (*settings)->add_header = 0;
        }
    }
    TRACE(TRACE_DEBUG, "settings->add_header: %d", (*settings)->add_header);

    (*settings)->max_size = g_key_file_get_integer(keyfile, "global", "max_size",NULL);
    TRACE(TRACE_DEBUG, "settings->max_size: %d", (*settings)->max_size);

    (*settings)->tls = g_key_file_get_integer(keyfile,"global","tls_enable",NULL);
    TRACE(TRACE_DEBUG, "settings->tls: %d", (*settings)->tls);

    (*settings)->tls_pass = g_key_file_get_string(keyfile,"global","tls_pass",NULL);
    TRACE(TRACE_DEBUG, "settings->tls_pass: %s", (*settings)->tls_pass);

    (*settings)->daemon = g_key_file_get_boolean(keyfile,"global","daemon",NULL);
    if (g_ascii_strcasecmp((*settings)->engine,"pipe") == 0) {
        TRACE(TRACE_ERR,"pipe engine can not be used in daemon mode");
        return -1;
    } 
    TRACE(TRACE_DEBUG, "settings->daemon: %d", (*settings)->daemon);

    if (g_key_file_has_group(keyfile,"sql")) {
        (*settings)->sql_driver = g_key_file_get_string(keyfile, "sql", "driver", NULL);
        (*settings)->sql_name = g_key_file_get_string(keyfile, "sql", "name", &error);
        if((*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if (g_ascii_strcasecmp((*settings)->backend[i],"sql") == 0) {
                    if ((*settings)->sql_name == NULL) {
                        TRACE(TRACE_ERR, "config error: %s", error->message);
                        g_error_free(error);
                        return -1;
                    }
                    break;
                }
            }
        }
        (*settings)->sql_host = g_key_file_get_string_list(keyfile, "sql", "host", &sql_num_hosts,NULL);
        (*settings)->sql_num_hosts = sql_num_hosts;
        (*settings)->sql_port = g_key_file_get_integer(keyfile, "sql", "port", NULL);
        (*settings)->sql_user = g_key_file_get_string(keyfile, "sql", "user", NULL);
        (*settings)->sql_pass = g_key_file_get_string(keyfile, "sql", "pass", NULL);
        (*settings)->sql_user_query = g_key_file_get_string(keyfile, "sql", "user_query", &error);
        if ((*settings)->sql_user_query == NULL) {
            TRACE(TRACE_ERR, "config error: %s", error->message);
            g_error_free(error);
            return -1;
        }

        (*settings)->sql_encoding = g_key_file_get_string(keyfile, "sql", "encoding", NULL);
        (*settings)->sql_max_connections = g_key_file_get_integer(keyfile, "sql", "max_connections", NULL);

        TRACE(TRACE_DEBUG, "settings->sql_driver: %s", (*settings)->sql_driver);
        TRACE(TRACE_DEBUG, "settings->sql_name: %s", (*settings)->sql_name);
        if ((*settings)->sql_host != NULL) {
            for(i = 0; (*settings)->sql_host[i] != NULL; i++) {
                TRACE(TRACE_DEBUG, "settings->sql_host: %s", (*settings)->sql_host[i]);
            }
        }
        TRACE(TRACE_DEBUG, "settings->sql_port: %d", (*settings)->sql_port);
        TRACE(TRACE_DEBUG, "settings->sql_user: %s", (*settings)->sql_user);
        TRACE(TRACE_DEBUG, "settings->sql_pass: %s", (*settings)->sql_pass);
        TRACE(TRACE_DEBUG, "settings->sql_user_query: %s", (*settings)->sql_user_query);
        TRACE(TRACE_DEBUG, "settings->sql_encoding: %s", (*settings)->sql_encoding);
        TRACE(TRACE_DEBUG, "settings->sql_max_connections: %d", (*settings)->sql_max_connections);
    } else {
        if((*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if (g_ascii_strcasecmp((*settings)->backend[i],"sql") == 0) {
                    TRACE(TRACE_ERR,"Can't find settings group sql");
                    return -1;
                }
            }
        }
    }

    if (g_key_file_has_group(keyfile,"ldap")) {
        (*settings)->ldap_uri = g_key_file_get_string(keyfile,"ldap","uri",NULL);
        (*settings)->ldap_host = g_key_file_get_string_list(keyfile, "ldap", "host", &ldap_num_hosts,NULL);
        (*settings)->ldap_num_hosts = ldap_num_hosts;

        if((*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if (g_ascii_strcasecmp((*settings)->backend[i],"ldap") == 0) {
                    if ((*settings)->ldap_uri == NULL && (*settings)->ldap_host == NULL) {
                        TRACE(TRACE_ERR, "config error: neither ldap uri nor ldap host supplied");
                        return -1;
                    }
                    break;
                }
            }
        }
        (*settings)->ldap_port = g_key_file_get_integer(keyfile,"ldap","port",NULL);
        if (!(*settings)->ldap_port)
            (*settings)->ldap_port = 389;
        (*settings)->ldap_binddn = g_key_file_get_string(keyfile,"ldap","binddn",NULL);
        (*settings)->ldap_bindpw = g_key_file_get_string(keyfile,"ldap","bindpw",NULL);

        (*settings)->ldap_base = g_key_file_get_string(keyfile,"ldap","base",&error);
        if((*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if ((g_ascii_strcasecmp((*settings)->backend[i],"ldap") == 0) && ((*settings)->ldap_base == NULL)) {
                    TRACE(TRACE_ERR, "config error: %s", error->message);
                    g_error_free(error);
                    return -1;
                }
            }
        }

        (*settings)->ldap_referrals = g_key_file_get_boolean(keyfile, "ldap","referrals",NULL);

        (*settings)->ldap_scope = g_key_file_get_string(keyfile, "ldap", "scope", NULL);
        if ((*settings)->ldap_scope != NULL && (*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if (g_ascii_strcasecmp((*settings)->backend[i],"ldap") == 0) {
                    if ((g_ascii_strcasecmp((*settings)->ldap_scope,"subtree") != 0) &&
                            (g_ascii_strcasecmp((*settings)->ldap_scope,"onelevel") != 0) &&
                            (g_ascii_strcasecmp((*settings)->ldap_scope,"base") != 0)) {
                        TRACE(TRACE_ERR, "invalid ldap scope");
                        return -1;
                    }
                }
            }
        } else {
            (*settings)->ldap_scope = g_strdup("subtree");
        }

        (*settings)->ldap_user_query = g_key_file_get_string(keyfile, "ldap", "user_query", &error);
        if ((*settings)->ldap_user_query == NULL) {
            TRACE(TRACE_ERR, "config error: %s", error->message);
            g_error_free(error);
            return -1;
        }

        TRACE(TRACE_DEBUG, "settings->ldap_uri: %s", (*settings)->ldap_uri);
        if ((*settings)->ldap_host != NULL) {
            for(i = 0; (*settings)->ldap_host[i] != NULL; i++) {
                TRACE(TRACE_DEBUG, "settings->ldap_host: %s", (*settings)->ldap_host[i]);
            }
        }
        TRACE(TRACE_DEBUG, "settings->ldap_port: %d", (*settings)->ldap_port);
        TRACE(TRACE_DEBUG, "settings->ldap_binddn: %s", (*settings)->ldap_binddn);
        TRACE(TRACE_DEBUG, "settings->ldap_bindpw: %s", (*settings)->ldap_bindpw);
        TRACE(TRACE_DEBUG, "settings->ldap_base: %s", (*settings)->ldap_base);
        TRACE(TRACE_DEBUG, "settings->ldap_referrals: %d", (*settings)->ldap_referrals);
        TRACE(TRACE_DEBUG, "settings->ldap_scope: %s", (*settings)->ldap_scope);
        TRACE(TRACE_DEBUG, "settings->ldap_user_query: %s", (*settings)->ldap_user_query);
    } else {
        if((*settings)->backend != NULL) {
            for (i=0; (*settings)->backend[i] != NULL; i++) {
                if (g_ascii_strcasecmp((*settings)->backend[i],"ldap") == 0) {
                    TRACE(TRACE_ERR,"Can't find settings group ldap");
                    return -1;
                }
            }
        }
    }
    
    /* smtpd group */
    (*settings)->nexthop_fail_code = g_key_file_get_integer(keyfile, "smtpd", "nexthop_fail_code", NULL);

    (*settings)->nexthop_fail_msg = g_key_file_get_string(keyfile, "smtpd", "nexthop_fail_msg", NULL);
    if ((*settings)->nexthop_fail_msg == NULL) {
        /* nexthop_fail_msg not configured, now we use default vaules */
        (*settings)->nexthop_fail_msg = g_strdup("Requested action aborted: local error in processing");
    }

    TRACE(TRACE_DEBUG, "settings->nexthop_fail_code: %d", (*settings)->nexthop_fail_code);
    TRACE(TRACE_DEBUG, "settings->nexthop_fail_msg: %s", (*settings)->nexthop_fail_msg);

    code_keys = g_key_file_get_keys(keyfile,"smtpd",&codes_length,NULL);
    while (codes_length--) {
        /* only insert smtp codes to hashtable */
        code = g_ascii_strtod(code_keys[codes_length],NULL);
        if ((code > 400) && (code < 600)) {
            code_msg = g_key_file_get_string(keyfile, "smtpd", code_keys[codes_length],NULL);
            smf_settings_set_smtp_code((*settings),code,code_msg);
            TRACE(TRACE_DEBUG,"settings->smtp_codes: append %d=%s",code,smf_settings_get_smtp_code((*settings),code));
            g_free(code_msg);
        }
    }
    g_strfreev(code_keys);
    g_key_file_free(keyfile);
    
    return 0;
}

int smf_settings_set_debug(SMFSettings_T *settings, int debug) {
    assert(settings);
    if ((debug != 0) && (debug != 1)) {
        TRACE(TRACE_ERR,"debug setting must be either 0 or 1");
        return -1;
    }
    
    settings->debug = debug;
    
    return 0;
}

int smf_settings_get_debug(SMFSettings_T *settings) {
    assert(settings);
    return settings->debug;
}

int smf_settings_set_config_file(SMFSettings_T *settings, char *cf) {
    assert(settings);
    assert(cf);

    if (!g_file_test(cf, G_FILE_TEST_EXISTS)) {
        TRACE(TRACE_ERR,"file [%s] does not exist.",cf);
        return -1;
    }
    
    if (settings->config_file != NULL)
        g_free(settings->config_file);
    settings->config_file = g_strdup(cf);
    
    return 0;
}

char *smf_settings_get_config_file(SMFSettings_T *settings) {
    assert(settings);
    return settings->config_file;
}

int smf_settings_set_queue_dir(SMFSettings_T *settings, char *qd) {
    assert(settings);    
    assert(qd);

    if (!g_file_test(qd, G_FILE_TEST_IS_DIR)) {
        TRACE(TRACE_ERR,"directory [%s] does not exist",qd);
        return -1;
    }
        
    if (g_access(qd,W_OK) != 0) {
        TRACE(TRACE_ERR,"directory [%s] is not writeable: %s (%d)",qd, strerror(errno), errno);
        return -1; 
    }
        
    if (settings->queue_dir != NULL)
        g_free(settings->queue_dir);
    
    settings->queue_dir = g_strdup(qd);
    
    return 0;
}

char *smf_settings_get_queue_dir(SMFSettings_T *settings) {
    assert(settings);
    return settings->queue_dir;
}

void smf_settings_set_engine(SMFSettings_T *settings, char *engine) {
    assert(settings);    
    assert(engine);

    if (settings->engine != NULL)
        g_free(settings->engine);
    settings->engine = g_strdup(engine);
}

char *smf_settings_get_engine(SMFSettings_T *settings) {
    assert(settings);
    return settings->engine;
}

void smf_settings_set_modules(SMFSettings_T *settings, char **modules) {
    assert(settings); 
    assert(*modules);

    if (settings->modules != NULL) {
        g_strfreev(settings->modules);
    }
    
    if (modules != NULL) {
        settings->modules = g_strdupv(modules);
    }
}

char **smf_settings_get_modules(SMFSettings_T *settings) {
    assert(settings);
    return settings->modules;
}

void smf_settings_set_nexthop(SMFSettings_T *settings, char *nexthop) {
    assert(settings);
    assert(nexthop);

    if (settings->nexthop != NULL) 
        g_free(settings->nexthop);
    
    settings->nexthop = g_strdup(nexthop);
}

char *smf_settings_get_nexthop(SMFSettings_T *settings) {
    assert(settings);
    return settings->nexthop;
}

void smf_settings_set_module_fail(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->module_fail = i;
}

int smf_settings_get_module_fail(SMFSettings_T *settings) {
    assert(settings);
    return settings->module_fail;
}

void smf_settings_set_nexthop_fail_code(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->nexthop_fail_code = i;
}

int smf_settings_get_nexthop_fail_code(SMFSettings_T *settings) {
    assert(settings);
    return settings->nexthop_fail_code;
}

void smf_settings_set_nexthop_fail_msg(SMFSettings_T *settings, char *msg) {
    assert(settings);
    assert(msg);

    if (settings->nexthop_fail_msg != NULL)
        g_free(settings->nexthop_fail_msg);
        
    settings->nexthop_fail_msg = g_strdup(msg);
}

char *smf_settings_get_nexthop_fail_msg(SMFSettings_T *settings) {
    assert(settings);
    return settings->nexthop_fail_msg;
}

void smf_settings_set_backend(SMFSettings_T *settings, char **backend) {
    assert(settings);
    assert(*backend);

    if (settings->backend != NULL)
        g_strfreev(settings->backend);
    
    if (backend != NULL) {
        settings->backend = g_strdupv(backend);
    }
}

char **smf_settings_get_backend(SMFSettings_T *settings) {
    assert(settings);
    return settings->backend;
}

void smf_settings_set_backend_connection(SMFSettings_T *settings, char *conn) {
    assert(settings);
    assert(conn);

    if (settings->backend_connection != NULL)
        g_free(settings->backend_connection);
        
    settings->backend_connection = g_strdup(conn);
}

char *smf_settings_get_backend_connection(SMFSettings_T *settings) {
    assert(settings);
    return settings->backend_connection;
}

void smf_settings_set_add_header(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->add_header = i;
}

int smf_settings_get_add_header(SMFSettings_T *settings) {
    assert(settings);
    return settings->add_header;
}

void smf_settings_set_max_size(SMFSettings_T *settings, unsigned long size) {
    assert(settings);
    settings->max_size = size;
}

unsigned long smf_settings_get_max_size(SMFSettings_T *settings) {
    assert(settings);
    return settings->max_size;
}

void smf_settings_set_tls(SMFSettings_T *settings, SMFTlsOption_T t) {
    assert(settings);
    settings->tls = t;
}

SMFTlsOption_T smf_settings_get_tls(SMFSettings_T *settings) {
    assert(settings);
    return settings->tls;
}

void smf_settings_set_tls_pass(SMFSettings_T *settings, char *pass) {
    assert(settings);
    assert(pass);

    if (settings->tls_pass != NULL)
        g_free(settings->tls_pass);
        
    settings->tls_pass = g_strdup(pass);
}

char *smf_settings_get_tls_pass(SMFSettings_T *settings) {
    assert(settings);
    return settings->tls_pass;
}

void smf_settings_set_daemon(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->daemon = i;
}

int smf_settings_get_daemon(SMFSettings_T *settings) {
    assert(settings);
    return settings->daemon;
}

void smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg) {
    char *strcode = g_strdup_printf("%d",code);

    assert(settings);
    assert(msg);

    g_hash_table_insert(settings-> smtp_codes, g_strdup(strcode), g_strdup(msg));
    free(strcode);
}

char *smf_settings_get_smtp_code(SMFSettings_T *settings, int code) {
    char *strcode = g_strdup_printf("%d",code);
    char *p = NULL;

    assert(settings);

    p = g_hash_table_lookup(settings->smtp_codes,strcode);
    g_free(strcode);

    return p;
}

void smf_settings_set_sql_driver(SMFSettings_T *settings, char *driver) {
    assert(settings);   
    assert(driver);

    if (settings->sql_driver != NULL)
        g_free(settings->sql_driver);
    
    settings->sql_driver = g_strdup(driver);
}

char *smf_settings_get_sql_driver(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_driver;
}

void smf_settings_set_sql_name(SMFSettings_T *settings, char *name) {
    assert(settings);
    assert(name);

    if (settings->sql_name != NULL)
        g_free(settings->sql_name);
        
    settings->sql_name = g_strdup(name);
}

char *smf_settings_get_sql_name(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_name;
}

void smf_settings_set_sql_host(SMFSettings_T *settings, char **host) {
    assert(settings);
    assert(*host);

    if (settings->sql_host != NULL)
        g_strfreev(settings->sql_host);
    
    if (host != NULL) {
        settings->sql_host = g_strdupv(host);
    }
    settings->sql_num_hosts = g_strv_length(host);
}

char **smf_settings_get_sql_host(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_host;
}

int smf_settings_get_sql_num_hosts(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_num_hosts;
}

void smf_settings_set_sql_port(SMFSettings_T *settings, int port) {
    assert(settings);
    settings->sql_port = port;
}

int smf_settings_get_sql_port(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_port;
}

void smf_settings_set_sql_user(SMFSettings_T *settings, char *user) {
    assert(settings);
    assert(user);

    if (settings->sql_user != NULL) 
        g_free(settings->sql_user);
        
    settings->sql_user = g_strdup(user);
}

char *smf_settings_get_sql_user(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_user;
}

void smf_settings_set_sql_pass(SMFSettings_T *settings, char *pass) {
    assert(settings);
    
    assert(pass);

    if (settings->sql_pass != NULL)
        g_free(settings->sql_pass);
    
    settings->sql_pass = g_strdup(pass);
} 

char *smf_settings_get_sql_pass(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_pass;
}

void smf_settings_set_sql_user_query(SMFSettings_T *settings, char *query) {
    assert(settings);
    assert(query);

    if (settings->sql_user_query != NULL) 
        g_free(settings->sql_user_query);
    
    settings->sql_user_query = g_strdup(query);
}

char *smf_settings_get_sql_user_query(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_user_query;
}

void smf_settings_set_sql_encoding(SMFSettings_T *settings, char *encoding) {
    assert(settings);
    assert(encoding);

    if (settings->sql_encoding != NULL)
        g_free(settings->sql_encoding);
        
    settings->sql_encoding = g_strdup(encoding);
}

char *smf_settings_get_sql_encoding(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_encoding;
}

void smf_settings_set_sql_max_connections(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->sql_max_connections = i;
}

int smf_settings_get_sql_max_connections(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_max_connections;
}

void smf_settings_set_ldap_uri(SMFSettings_T *settings, char *uri) {
    assert(settings);
    assert(uri);

    if (settings->ldap_uri != NULL) 
        g_free(settings->ldap_uri);
        
    settings->ldap_uri = g_strdup(uri);
}

char *smf_settings_get_ldap_uri(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_uri;
}

void smf_settings_set_ldap_host(SMFSettings_T *settings, char **host) {
    assert(settings);
    assert(*host);

    if (settings->ldap_host != NULL)
        g_strfreev(settings->ldap_host);
    
    if (host != NULL) {
        settings->ldap_host = g_strdupv(host);
    }
    settings->ldap_num_hosts = g_strv_length(host);
}

char **smf_settings_get_ldap_host(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_host;
}

int smf_settings_get_ldap_num_hosts(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_num_hosts;
}

void smf_settings_set_ldap_port(SMFSettings_T *settings, int port) {
    assert(settings);
    settings->ldap_port = port;
}

int smf_settings_get_ldap_port(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_port;
}

void smf_settings_set_ldap_binddn(SMFSettings_T *settings, char *binddn) {
    assert(settings);
    assert(binddn);

    if (settings->ldap_binddn != NULL)
        g_free(settings->ldap_binddn);
        
    settings->ldap_binddn = g_strdup(binddn);
}

char *smf_settings_get_ldap_binddn(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_binddn;
}

void smf_settings_set_ldap_bindpw(SMFSettings_T *settings, char *bindpw) {
    assert(settings);   
    assert(bindpw);

    if (settings->ldap_bindpw != NULL)
        g_free(settings->ldap_bindpw);
        
    settings->ldap_bindpw = g_strdup(bindpw);
}

char *smf_settings_get_ldap_bindpw(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_bindpw;
}

void smf_settings_set_ldap_base(SMFSettings_T *settings, char *base) {
    assert(settings);
    assert(base);

    if (settings->ldap_base != NULL)
        g_free(settings->ldap_base);
        
    settings->ldap_base = g_strdup(base);
}

char *smf_settings_get_ldap_base(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_base;
}

void smf_settings_set_ldap_referrals(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->ldap_referrals = i;
}

int smf_settings_get_ldap_referrals(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_referrals;
}

void smf_settings_set_ldap_scope(SMFSettings_T *settings, char *scope) {
    assert(settings);
    assert(scope);

    if (settings->ldap_scope != NULL)
        g_free(settings->ldap_scope);
        
    settings->ldap_scope = g_strdup(scope);
}

char *smf_settings_get_ldap_scope(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_scope;
}

void smf_settings_set_ldap_user_query(SMFSettings_T *settings, char *query) {
    assert(settings);
    assert(query);

    if (settings->ldap_user_query != NULL)
        g_free(settings->ldap_user_query);
        
    settings->ldap_user_query = g_strdup(query);
}

char *smf_settings_get_ldap_user_query(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_user_query;
}
 