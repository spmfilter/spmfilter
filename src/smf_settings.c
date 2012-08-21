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
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/stat.h>

#if 0
#include <glib.h>
#include <glib/gstdio.h>
#endif

#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_settings_private.h"
#include "smf_list.h"
#include "smf_dict.h"
#include "smf_core.h"
#include "smf_internal.h"

#define MAX_LINE 200

/**
 * This enum stores the status for each parsed line (internal use only).
 */
typedef enum _line_status {
    LINE_UNPROCESSED,
    LINE_ERROR,
    LINE_EMPTY,
    LINE_COMMENT,
    LINE_SECTION,
    LINE_VALUE
} line_status ;

int _get_boolean(char *val) {
    int ret = 0;

    if (val[0]=='y' || val[0]=='Y' || val[0]=='1' || val[0]=='t' || val[0]=='T') {
        ret = 1 ;
    } 

    return ret;
}

int _get_integer(char *val) {
    return (int)strtol(val, NULL, 0);
}

static line_status _parse_line(
        char *input_line,
        char *section,
        char *key,
        char *value) {   
    
    line_status sta ;
    char *line;
    int len;

    line = smf_core_strstrip(input_line);
    len = (int)strlen(line);

    sta = LINE_UNPROCESSED ;
    if (len<1) {
        /* Empty line */
        sta = LINE_EMPTY ;
    } else if (line[0]=='#' || line[0]==';') {
        /* Comment line */
        sta = LINE_COMMENT ; 
    } else if (line[0]=='[' && line[len-1]==']') {
        /* Section name */
        sscanf(line, "[%[^]]", section);
        section = smf_core_strstrip(section);
        section = smf_core_strlwc(section);
        sta = LINE_SECTION ;
    } else if (sscanf (line, "%[^=] = \"%[^\"]\"", key, value) == 2
           ||  sscanf (line, "%[^=] = '%[^\']'",   key, value) == 2
           ||  sscanf (line, "%[^=] = %[^#]",     key, value) == 2) {
        /* Usual key=value, with or without comments */
        key = smf_core_strstrip(key);
        key = smf_core_strlwc(key);
        value = smf_core_strstrip(value);
        /*
         * sscanf cannot handle '' or "" as empty values
         * this is done here
         */
        if (!strcmp(value, "\"\"") || (!strcmp(value, "''"))) {
            value[0]=0 ;
        }
        sta = LINE_VALUE ;
    } else if (sscanf(line, "%[^=] = %[;#]", key, value)==2
           ||  sscanf(line, "%[^=] %[=]", key, value) == 2) {
        /*
         * Special cases:
         * key=
         * key=;
         * key=#
         */
        key = smf_core_strstrip(key);
        key = smf_core_strlwc(key);
        value[0]=0 ;
        sta = LINE_VALUE ;
    } else {
        /* Generate syntax error */
        sta = LINE_ERROR ;
    }
    return sta ;
}

void _set_config_value(SMFSettings_T **settings, char *section, char *key, char *val) {
    char **sl = NULL;
    char **p = NULL;
    char *s = NULL;
    int i;

    if (val==NULL || strlen(val) == 0)
        return;

    /** [global]debug **/
    if (strcmp(key,"debug")==0) {
        (*settings)->debug = _get_boolean(val);
        configure_debug((*settings)->debug);
    /** [global]queue_dir **/
    } else if (strcmp(key,"queue_dir")==0) {
        if ((*settings)->queue_dir!=NULL)
            free((*settings)->queue_dir);

        (*settings)->queue_dir = strdup(val);
    /** [global]modules **/
    } else if (strcmp(key, "modules")==0) {
        if (smf_list_size((*settings)->modules) > 0) {
            if (smf_list_free((*settings)->modules)!=0)
                TRACE(TRACE_ERR,"failed to free modules list");
            else 
                if (smf_list_new(&((*settings)->modules),_string_list_destroy)!=0)
                    TRACE(TRACE_ERR,"failed to create modules list");
        }
        sl = smf_core_strsplit(val, ";");
        p = sl;
        while(*p != NULL) {
            s = smf_core_strstrip(*p);
            smf_list_append((*settings)->modules, s);
            p++;
        }
        free(sl);
    /** [global]engine **/
    } else if (strcmp(key,"engine")==0) {
        if ((*settings)->engine!=NULL) 
            free((*settings)->engine);

        (*settings)->engine = strdup(val);
    /** [global]module_fail **/
    } else if (strcmp(key,"module_fail")==0) {
        i = _get_integer(val);

        /** check allowed values... */
        if (i==1 || i==2 || i==3)
            (*settings)->module_fail = i;
    /** [global]nexthop **/
    } else if (strcmp(key,"nexthop")==0) {
        if ((*settings)->nexthop!=NULL)
            free((*settings)->nexthop);
    
        (*settings)->nexthop = strdup(val);
    /** [global]backend **/
    } else if (strcmp(key,"backend")==0) {
        if ((*settings)->backend!=NULL)
            free((*settings)->backend);

        if ((strcmp(val,"sql")==0)||(strcmp(val,"ldap")==0))
            (*settings)->backend = strdup(val);
    /** [global]backend_connection **/
    } else if (strcmp(key,"backend_connection")==0) {
        if ((*settings)->backend_connection!=NULL) 
            free((*settings)->backend_connection);

        if ((strcmp(val,"balance")==0)||(strcmp(val,"failover")==0)) 
            (*settings)->backend_connection = strdup(val);
    /** [global]add_header **/
    } else if (strcmp(key,"add_header")==0) {
        (*settings)->add_header = _get_boolean(val);
    /** [global]max_size **/
    } else if (strcmp(key,"max_size")==0) {
        (*settings)->max_size = _get_integer(val);
    /** [global]tls_enable **/
    } else if (strcmp(key,"tls_enable")==0) {
        i = _get_integer(val);
        /** check allowed values... */
        if (i==0 || i==1 || i==2)
            (*settings)->tls = i;
    /** [global]tls_pass **/
    } else if (strcmp(key,"tls_pass")==0) {
        if ((*settings)->tls_pass!=NULL)
            free((*settings)->tls_pass);
    
        (*settings)->tls_pass = strdup(val);
    /** [global]lib_dir **/
    } else if (strcmp(key,"lib_dir")==0) {
        if ((*settings)->lib_dir!=NULL)
            free((*settings)->lib_dir);
    
        (*settings)->lib_dir = strdup(val);
    }
}

/* has to be removed */
SMFSettings_T *smf_settings_new(void) {
    SMFSettings_T *settings = NULL;

    if (!(settings = (SMFSettings_T *)calloc(1, sizeof(SMFSettings_T))))
        return NULL;

    settings->debug = 0;
    settings->config_file = NULL;
    settings->queue_dir = NULL;
    settings->engine = NULL;
    if (smf_list_new(&settings->modules, _string_list_destroy) != 0) {
        TRACE(TRACE_ERR,"failed to allocate space for settings->modules");
        free(settings);
        return NULL;
    }
    settings->nexthop = NULL;
    settings->nexthop_fail_msg = NULL;
    settings->backend = NULL;
    settings->backend_connection = NULL;
    settings->tls_pass = NULL;
    settings->lib_dir = NULL;
    settings->smtp_codes = smf_dict_new();
    settings->sql_driver = NULL;
    settings->sql_name = NULL;
    if (smf_list_new(&settings->sql_host, _string_list_destroy) != 0) {
        TRACE(TRACE_ERR,"failed to allocate space for settings->sql_host");
        smf_list_free(settings->modules);
        free(settings);
        return NULL;
    }
    settings->sql_user = NULL;
    settings->sql_pass = NULL;
    settings->sql_user_query = NULL;
    settings->sql_encoding = NULL;
    settings->ldap_uri = NULL;
    if (smf_list_new(&settings->ldap_host, _string_list_destroy) != 0) {
        TRACE(TRACE_ERR,"failed to allocate space for settings->ldap_host");
        smf_list_free(settings->modules);
        smf_list_free(settings->ldap_host);
        free(settings);
        return NULL;
    }
    settings->ldap_binddn = NULL;
    settings->ldap_bindpw = NULL;
    settings->ldap_base = NULL;
    settings->ldap_scope = NULL;
    settings->ldap_referrals = 0;
    settings->ldap_user_query = NULL;
    settings->module_fail = 3;
    settings->nexthop_fail_code = 451;
    settings->add_header = 1;
    settings->max_size = 0;
    settings->tls = 0;
    settings->daemon = 0;
    settings->sql_max_connections = 3;
    settings->sql_port = 0;
    settings->ldap_connection = NULL;
    
    return settings;
}

void smf_settings_free(SMFSettings_T *settings) {
    assert(settings);

    if (smf_list_free(settings->modules) != 0)
        TRACE(TRACE_ERR,"failed to free settings->modules");
    
    if (settings->config_file != NULL) free(settings->config_file);
    if (settings->queue_dir != NULL) free(settings->queue_dir);
    if (settings->engine != NULL) free(settings->engine);
    if (settings->nexthop != NULL) free(settings->nexthop);
    if (settings->nexthop_fail_msg != NULL) free(settings->nexthop_fail_msg);
    if (settings->backend != NULL) free(settings->backend);
    if (settings->backend_connection != NULL) free(settings->backend_connection);
    if (settings->tls_pass != NULL) free(settings->tls_pass);
    if (settings->lib_dir != NULL) free(settings->lib_dir);
    smf_dict_free(settings->smtp_codes);
    if (settings->sql_driver) free(settings->sql_driver);
    if (settings->sql_name) free(settings->sql_name);
    if (smf_list_free(settings->sql_host) != 0)
        TRACE(TRACE_ERR,"failed to free settings->sql_host");
    if (settings->sql_user != NULL) free(settings->sql_user);
    if (settings->sql_pass != NULL) free(settings->sql_pass);
    if (settings->sql_user_query != NULL) free(settings->sql_user_query);
    if (settings->sql_encoding != NULL) free(settings->sql_encoding);
    if (settings->ldap_uri != NULL) free(settings->ldap_uri); 
    if (smf_list_free(settings->ldap_host) != 0)
        TRACE(TRACE_ERR,"failed to free settings->ldap_host");
    if (settings->ldap_binddn != NULL) free(settings->ldap_binddn);
    if (settings->ldap_bindpw != NULL) free(settings->ldap_bindpw);
    if (settings->ldap_base != NULL) free(settings->ldap_base);
    if (settings->ldap_scope != NULL) free(settings->ldap_scope);
    if (settings->ldap_user_query != NULL) free(settings->ldap_user_query);
    
    free(settings);
}

int smf_settings_parse_config(SMFSettings_T **settings, char *alternate_file) {
    FILE *in = NULL;
    char line[MAX_LINE+1];
    char section[MAX_LINE+1];
    char key[MAX_LINE+1];
    char val[MAX_LINE+1];
    SMFListElem_T *elem = NULL;
    char *s = NULL;
    int last=0;
    int len=0;
    int lineno=0;
    int errs=0;

    assert(*settings);
    assert(alternate_file);

    /* fallback to default config path,
     * if config file is not defined as
     * command argument */
    if (alternate_file != NULL) {
        (*settings)->config_file = strdup(alternate_file);
    } else {
        (*settings)->config_file = strdup("/etc/spmfilter.conf");
    }

    if ((in=fopen((*settings)->config_file, "r")) == NULL) {
        TRACE(TRACE_ERR,"Error loading config: %s (%d)",strerror(errno), errno);
        return -1;
    }

    memset(line, 0, MAX_LINE);
    memset(section, 0, MAX_LINE);
    memset(key, 0, MAX_LINE);
    memset(val, 0, MAX_LINE);

    while (fgets(line+last, MAX_LINE-last, in) != NULL) {
        lineno++ ;
        len = (int)strlen(line)-1;
        if (len==0)
            continue;

        if (line[len]!='\n') {
            TRACE(TRACE_ERR,"input line too long in %s (%d)\n", (*settings)->config_file, lineno);
            fclose(in);
            return -1;
        }

        /* Detect multi-line */
        if (line[len]=='\\') {
            /* Multi-line value */
            last=len;
            continue;
        } else {
            last=0;
        }

        switch (_parse_line(line, section, key, val)) {
            case LINE_EMPTY:
            case LINE_COMMENT:
            break;

            case LINE_SECTION:
            
            //errs = dictionary_set(dict, section, NULL);
            //printf("SECION [%s]\n",section);
            break;

            case LINE_VALUE:
            _set_config_value(settings,section,key,val);
            //printf("%s:%s=>%s\n",section,key,val);
            //sprintf(tmp, "%s:%s", section, key);
            //errs = dictionary_set(dict, tmp, val) ;
            break;

            case LINE_ERROR:
            TRACE(TRACE_ERR, "syntax error in %s (%d): %s",
                    (*settings)->config_file,
                    lineno,line);
            errs++;
            break;

            default:
            break;
        }
        memset(line, 0, MAX_LINE);
        last=0;
        if (errs<0) {
            fprintf(stderr, "memory allocation failure\n");
            break ;
        }
    }

    if (fclose(in)!=0)
        TRACE(TRACE_ERR,"failed to close config file: %s (%d)",strerror(errno), errno);

    // check defaults
    if ((*settings)->queue_dir == NULL) {
        (*settings)->queue_dir = strdup("/var/spool/spmfilter");
        TRACE(TRACE_DEBUG,"config value queue_dir not set, using default");
    }

    if ((*settings)->nexthop == NULL) {
        TRACE(TRACE_ERR,"config value nexthop not set");
        return -1;
    }

    if ((*settings)->engine == NULL) {
        TRACE(TRACE_ERR,"config value engine not set");
        return -1;
    }

    if ((*settings)->backend_connection == NULL) {
        (*settings)->backend_connection = strdup("failover");
        TRACE(TRACE_DEBUG,"config value backend_connection not set, using default");
    }

    TRACE(TRACE_DEBUG, "settings->queue_dir: [%s]", (*settings)->queue_dir);
    TRACE(TRACE_DEBUG, "settings->engine: [%s]", (*settings)->engine);
    elem = smf_list_head((*settings)->modules);
    while(elem != NULL) {
        s = (char *)smf_list_data(elem);
        TRACE(TRACE_DEBUG, "settings->modules: [%s]", s);
        elem = elem->next;
    }
    TRACE(TRACE_DEBUG, "settings->module_fail [%d]",(*settings)->module_fail);
    TRACE(TRACE_DEBUG, "settings->nexthop: [%s]", (*settings)->nexthop);
    TRACE(TRACE_DEBUG, "settings->backend: [%s]", (*settings)->backend);
    TRACE(TRACE_DEBUG, "settings->backend_connection: [%s]", (*settings)->backend_connection);
    TRACE(TRACE_DEBUG, "settings->add_header: [%d]", (*settings)->add_header);
    TRACE(TRACE_DEBUG, "settings->max_size: [%d]", (*settings)->max_size);
    TRACE(TRACE_DEBUG, "settings->tls: [%d]", (*settings)->tls);
    TRACE(TRACE_DEBUG, "settings->tls_pass: [%s]", (*settings)->tls_pass);
    TRACE(TRACE_DEBUG, "settings->lib_dir: [%s]", (*settings)->lib_dir);

    return 0;
#if 0
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
    configure_debug((*settings)->debug);
    
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

    (*settings)->lib_dir = g_key_file_get_string(keyfile,"global","lib_dir",NULL);
    TRACE(TRACE_DEBUG, "settings->lib_dir: %s", (*settings)->lib_dir);

    (*settings)->daemon = g_key_file_get_boolean(keyfile,"global","daemon",NULL);
    if ((g_ascii_strcasecmp((*settings)->engine,"pipe") == 0) && ((*settings)->daemon == 1)) {
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
#endif 
    return 0;
}

int smf_settings_set_debug(SMFSettings_T *settings, int debug) {
    assert(settings);
    if ((debug != 0) && (debug != 1)) {
        TRACE(TRACE_ERR,"debug setting must be either 0 or 1");
        return -1;
    }
    configure_debug(debug);
    settings->debug = debug;
    
    return 0;
}

int smf_settings_get_debug(SMFSettings_T *settings) {
    assert(settings);
    return settings->debug;
}

int smf_settings_set_config_file(SMFSettings_T *settings, char *cf) {
    struct stat sb;
    assert(settings);
    assert(cf);

    if (stat(cf,&sb) != 0) {
        TRACE(TRACE_ERR,"file [%s] does not exist: %s (%d)",cf,strerror(errno), errno);
        return -1; 
    }
    
    if (settings->config_file != NULL) free(settings->config_file);
    settings->config_file = strdup(cf);
    
    return 0;
}

char *smf_settings_get_config_file(SMFSettings_T *settings) {
    assert(settings);
    return settings->config_file;
}

int smf_settings_set_queue_dir(SMFSettings_T *settings, char *qd) {
    struct stat sb;
    assert(settings);    
    assert(qd);

    if (stat(qd,&sb) != 0) {
        TRACE(TRACE_ERR,"directory [%s] does not exist: %s (%d)",qd,strerror(errno), errno);
        return -1; 
    }

    if(!S_ISDIR(sb.st_mode)) {
        TRACE(TRACE_ERR,"[%s] is not a directory",qd);
        return(-2); 
    }
        
    if (access(qd,W_OK) != 0) {
        TRACE(TRACE_ERR,"directory [%s] is not writeable: %s (%d)",qd, strerror(errno), errno);
        return -1; 
    }
        
    if (settings->queue_dir != NULL)
        free(settings->queue_dir);
    
    settings->queue_dir = strdup(qd);
    
    return 0;
}

char *smf_settings_get_queue_dir(SMFSettings_T *settings) {
    assert(settings);
    return settings->queue_dir;
}

void smf_settings_set_engine(SMFSettings_T *settings, char *engine) {
    assert(settings);    
    assert(engine);

    if (settings->engine != NULL) free(settings->engine);
    settings->engine = strdup(engine);
}

char *smf_settings_get_engine(SMFSettings_T *settings) {
    assert(settings);
    return settings->engine;
}

int smf_settings_add_module(SMFSettings_T *settings, char *module) {
    assert(settings); 
    assert(module);
   
    return smf_list_append(settings->modules, (void *)module);
}

SMFList_T *smf_settings_get_modules(SMFSettings_T *settings) {
    assert(settings);
    return settings->modules;
}

void smf_settings_set_nexthop(SMFSettings_T *settings, char *nexthop) {
    assert(settings);
    assert(nexthop);

    if (settings->nexthop != NULL) free(settings->nexthop);
    
    settings->nexthop = strdup(nexthop);
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

    if (settings->nexthop_fail_msg != NULL) free(settings->nexthop_fail_msg);
        
    settings->nexthop_fail_msg = strdup(msg);
}

char *smf_settings_get_nexthop_fail_msg(SMFSettings_T *settings) {
    assert(settings);
    return settings->nexthop_fail_msg;
}

void smf_settings_set_backend(SMFSettings_T *settings, char *backend) {
    assert(settings);
    assert(backend);

    if (settings->backend != NULL) free(settings->backend);
        
    settings->backend = strdup(backend);
}

char *smf_settings_get_backend(SMFSettings_T *settings) {
    assert(settings);
    return settings->backend;
}

void smf_settings_set_backend_connection(SMFSettings_T *settings, char *conn) {
    assert(settings);
    assert(conn);

    if (settings->backend_connection != NULL) free(settings->backend_connection);
        
    settings->backend_connection = strdup(conn);
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

    if (settings->tls_pass != NULL) free(settings->tls_pass);
        
    settings->tls_pass = strdup(pass);
}

char *smf_settings_get_tls_pass(SMFSettings_T *settings) {
    assert(settings);
    return settings->tls_pass;
}

void smf_settings_set_lib_dir(SMFSettings_T *settings, char *lib_dir) {
    assert(settings);
    assert(lib_dir);

    if (settings->lib_dir != NULL) free(settings->lib_dir);
        
    settings->lib_dir = strdup(lib_dir);
}

char *smf_settings_get_lib_dir(SMFSettings_T *settings) {
    assert(settings);
    return settings->lib_dir;
}

void smf_settings_set_daemon(SMFSettings_T *settings, int i) {
    assert(settings);
    settings->daemon = i;
}

int smf_settings_get_daemon(SMFSettings_T *settings) {
    assert(settings);
    return settings->daemon;
}

int smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg) {
    char *strcode = NULL;
    int res;

    assert(settings);
    assert(msg);

    asprintf(&strcode,"%d",code);

    res = smf_dict_set(settings->smtp_codes,strcode,msg);
    free(strcode);
    return res;
}

char *smf_settings_get_smtp_code(SMFSettings_T *settings, int code) {
    char *strcode = NULL;
    char *p = NULL;

    assert(settings);

    asprintf(&strcode,"%d",code);
    p = smf_dict_get(settings->smtp_codes,strcode);
    free(strcode);

    return p;
}

void smf_settings_set_sql_driver(SMFSettings_T *settings, char *driver) {
    assert(settings);   
    assert(driver);

    if (settings->sql_driver != NULL) free(settings->sql_driver);
    
    settings->sql_driver = strdup(driver);
}

char *smf_settings_get_sql_driver(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_driver;
}

void smf_settings_set_sql_name(SMFSettings_T *settings, char *name) {
    assert(settings);
    assert(name);

    if (settings->sql_name != NULL) free(settings->sql_name);
        
    settings->sql_name = strdup(name);
}

char *smf_settings_get_sql_name(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_name;
}

int smf_settings_add_sql_host(SMFSettings_T *settings, char *host) {
    assert(settings);
    assert(host);

    return smf_list_append(settings->sql_host,(void *)host);
}

SMFList_T *smf_settings_get_sql_host(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_host;
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

    if (settings->sql_user != NULL) free(settings->sql_user);
        
    settings->sql_user = strdup(user);
}

char *smf_settings_get_sql_user(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_user;
}

void smf_settings_set_sql_pass(SMFSettings_T *settings, char *pass) {
    assert(settings);
    assert(pass);

    if (settings->sql_pass != NULL) free(settings->sql_pass);
    
    settings->sql_pass = strdup(pass);
} 

char *smf_settings_get_sql_pass(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_pass;
}

void smf_settings_set_sql_user_query(SMFSettings_T *settings, char *query) {
    assert(settings);
    assert(query);

    if (settings->sql_user_query != NULL) free(settings->sql_user_query);
    
    settings->sql_user_query = strdup(query);
}

char *smf_settings_get_sql_user_query(SMFSettings_T *settings) {
    assert(settings);
    return settings->sql_user_query;
}

void smf_settings_set_sql_encoding(SMFSettings_T *settings, char *encoding) {
    assert(settings);
    assert(encoding);

    if (settings->sql_encoding != NULL) free(settings->sql_encoding);
        
    settings->sql_encoding = strdup(encoding);
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

    if (settings->ldap_uri != NULL) free(settings->ldap_uri);
        
    settings->ldap_uri = strdup(uri);
}

char *smf_settings_get_ldap_uri(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_uri;
}

int smf_settings_add_ldap_host(SMFSettings_T *settings, char *host) {
    assert(settings);
    assert(host);

    return smf_list_append(settings->ldap_host, (void *)host);
}

SMFList_T *smf_settings_get_ldap_host(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_host;
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

    if (settings->ldap_binddn != NULL) free(settings->ldap_binddn);
        
    settings->ldap_binddn = strdup(binddn);
}

char *smf_settings_get_ldap_binddn(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_binddn;
}

void smf_settings_set_ldap_bindpw(SMFSettings_T *settings, char *bindpw) {
    assert(settings);   
    assert(bindpw);
    
    if (settings->ldap_bindpw != NULL) free(settings->ldap_bindpw);
        
    settings->ldap_bindpw = strdup(bindpw);
}

char *smf_settings_get_ldap_bindpw(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_bindpw;
}

void smf_settings_set_ldap_base(SMFSettings_T *settings, char *base) {
    assert(settings);
    assert(base);

    if (settings->ldap_base != NULL) free(settings->ldap_base);
        
    settings->ldap_base = strdup(base);
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

    if (settings->ldap_scope != NULL) free(settings->ldap_scope);
        
    settings->ldap_scope = strdup(scope);
}

char *smf_settings_get_ldap_scope(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_scope;
}

void smf_settings_set_ldap_user_query(SMFSettings_T *settings, char *query) {
    assert(settings);
    assert(query);

    if (settings->ldap_user_query != NULL) free(settings->ldap_user_query);
        
    settings->ldap_user_query = strdup(query);
}

char *smf_settings_get_ldap_user_query(SMFSettings_T *settings) {
    assert(settings);
    return settings->ldap_user_query;
}
 
