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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/smf_list.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#include "test.h"
#if 0
#define TEST_ENGINE "smtpd"
#define TEST_STRING_1 "String1"
#define TEST_STRING_2 "String2"
#define TEST_CONFIG_FILE "test_settings"
#define TEST_QUEUE_DIR "/tmp"
#define TEST_NEXTHOP "localhost:10025"
#define TEST_FAIL_MSG "processing aborted"
#define TEST_BACKEND_CONN "failover"
#define TEST_PASS "test"
#define TEST_SQL_DRIVER "mysql"
#define TEST_SQL_NAME "maildb"
#define TEST_HOST "localhost"
#define TEST_USER "username"
#define TEST_SQL_QUERY "select * from test"
#define TEST_ENCODING "UTF-8"
#define TEST_LDAP_BASE "dc=example,dc=com"
#define TEST_LDAP_SCOPE "subtree"
#define TEST_LDAP_QUERY "(objectClass=*)"

#define THIS_MODULE "bla"
#endif

int main (int argc, char const *argv[]) {
    SMFSettings_T *settings = NULL;
    SMFSettingsGroup_T *test_group = NULL;
    char *s = NULL;
    SMFList_T *list = NULL;

    printf("Start SMFSettings_T tests...\n");
    printf("* testing smf_settings_new()...\t\t\t\t");
    settings = smf_settings_new();      
    if (settings != NULL)
        printf("passed\n");
    else {
        printf("failed\n");
        return -1;
    }
    
    smf_settings_set_debug(settings, 1);

    printf("* testing smf_settings_parse_config()...\t\t");
    if (smf_settings_parse_config(&settings,"../../spmfilter.conf.sample") != 0) {
        printf("failed\n");
        return -1;
    } else 
        printf("passed\n");

    printf("* testing smf_settings_set_debug()...\t\t\t");
    if (smf_settings_set_debug(settings,0) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }

    printf("* testing smf_settings_set_config_file()...\t\t");
    if (smf_settings_set_config_file(settings,test_config_file) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_settings_get_config_file()...\t\t");
    if (strcmp(smf_settings_get_config_file(settings),test_config_file) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
        smf_settings_set_config_file(settings, "../../spmfilter.conf.sample");
    }
    
    printf("* testing smf_settings_set_queue_dir()...\t\t");
    if (smf_settings_set_queue_dir(settings, test_queue_dir) != 0) {
        printf("failed\n");
        return -1;
    } 
    printf("passed\n");

    printf("* testing smf_settings_get_queue_dir()...\t\t");
    if (strcmp(smf_settings_get_queue_dir(settings),test_queue_dir) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_engine()...\t\t\t");
    smf_settings_set_engine(settings, test_string);
    printf("passed\n");

    printf("* testing smf_settings_get_engine()...\t\t\t");
    if (strcmp(smf_settings_get_engine(settings),test_string) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    

    printf("* testing smf_settings_add_module()...\t\t\t");
    s = strdup(test_module);
    if (smf_settings_add_module(settings,s)!=0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }

    printf("* testing smf_settings_get_modules()...\t\t\t");
    list = smf_settings_get_modules(settings);
    if (smf_list_size(list)!=3) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");


    printf("* testing smf_settings_set_module_fail()...\t\t");
    smf_settings_set_module_fail(settings,2);
    printf("passed\n");

    printf("* testing smf_settings_get_module_fail()...\t\t");
    if (smf_settings_get_module_fail(settings) == 2) {
        printf("passed\n");
    } else {
        printf("failed\n");
        return -1;
    }
    
    printf("* testing smf_settings_set_nexthop()...\t\t\t");
    smf_settings_set_nexthop(settings, test_ip);
    printf("passed\n");

    printf("* testing smf_settings_get_nexthop()...\t\t\t");
    if(strcmp(smf_settings_get_nexthop(settings),test_ip) != 0) {
        printf("failed\n");
        return -1;
    } 
    printf("passed\n");

    printf("* testing smf_settings_set_nexthop_fail_code()...\t");
    smf_settings_set_nexthop_fail_code(settings, 450);
    printf("passed\n");

    printf("* testing smf_settings_get_nexthop_fail_code()...\t");
    if(smf_settings_get_nexthop_fail_code(settings) != 450) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
      
    printf("* testing smf_settings_set_nexthop_fail_msg()...\t");
    smf_settings_set_nexthop_fail_msg(settings, test_string);
    printf("passed\n");
    
    printf("* testing smf_settings_get_nexthop_fail_msg()...\t");
    if(strcmp(smf_settings_get_nexthop_fail_msg(settings),test_string) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
       

    printf("* testing smf_settings_set_backend()...\t\t\t");
    smf_settings_set_backend(settings,test_backend);
    printf("passed\n");

    printf("* testing smf_settings_get_backend()...\t\t\t");
    if(strcmp(smf_settings_get_backend(settings),test_backend) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");


    printf("* testing smf_settings_set_backend_connection()...\t");
    smf_settings_set_backend_connection(settings, test_backend_conn);
    printf("passed\n");

    printf("* testing smf_settings_get_backend_connection()...\t");
    if(strcmp(smf_settings_get_backend_connection(settings),test_backend_conn) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    
    printf("* testing smf_settings_set_add_header()...\t\t");
    smf_settings_set_add_header(settings, 1);
    printf("passed\n");

    printf("* testing smf_settings_get_add_header()...\t\t");
    if(smf_settings_get_add_header(settings) != 1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    
    printf("* testing smf_settings_set_max_size()...\t\t");
    smf_settings_set_max_size(settings, 5000);
    printf("passed\n");

    printf("* testing smf_settings_get_max_size()...\t\t");
    if(smf_settings_get_max_size(settings) != 5000) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_tls()...\t\t\t");
    smf_settings_set_tls(settings, SMF_TLS_REQUIRED);
    printf("passed\n");
    
    printf("* testing smf_settings_get_tls()...\t\t\t");
    if (smf_settings_get_tls(settings) != SMF_TLS_REQUIRED) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
   
    printf("* testing smf_settings_set_tls_pass()...\t\t");
    smf_settings_set_tls_pass(settings, test_string);
    printf("passed\n");

    printf("* testing smf_settings_get_tls_pass()...\t\t");
    if(strcmp(smf_settings_get_tls_pass(settings),test_string) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_lib_dir()...\t\t\t");
    smf_settings_set_lib_dir(settings, test_queue_dir);
    printf("passed\n");

    printf("* testing smf_settings_get_lib_dir()...\t\t\t");
    if(strcmp(smf_settings_get_lib_dir(settings),test_queue_dir) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_daemon()...\t\t\t");
    smf_settings_set_daemon(settings, 1);
    printf("passed\n");

    printf("* testing smf_settings_get_daemon()...\t\t\t");
    if (smf_settings_get_daemon(settings) != 1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
   
    printf("* testing smf_settings_set_sql_driver()...\t\t");
    smf_settings_set_sql_driver(settings, test_sql_driver);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_driver()...\t\t");
    if (strcmp(smf_settings_get_sql_driver(settings),test_sql_driver) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_sql_name()...\t\t");
    smf_settings_set_sql_name(settings, test_sql_name);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_name()...\t\t");
    if (strcmp(smf_settings_get_sql_name(settings),test_sql_name) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n"); 

    printf("* testing smf_settings_add_sql_host()...\t\t");
    s = strdup(test_ip);
    smf_settings_add_sql_host(settings, s);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_hosts()...\t\t");
    list = smf_settings_get_sql_hosts(settings);
    if (smf_list_size(list)!=1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

#if 0
    printf("* testing smf_settings_set_sql_port()...\t\t");
    smf_settings_set_sql_port(settings, 1234);
    if (smf_settings_get_sql_port(settings) != 1234) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_sql_user()...\t\t");
    smf_settings_set_sql_user(settings, TEST_USER);
    if (strcmp(smf_settings_get_sql_user(settings),TEST_USER) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_sql_pass()...\t\t");
    smf_settings_set_sql_pass(settings, TEST_PASS);
    if (strcmp(smf_settings_get_sql_pass(settings),TEST_PASS) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_sql_user_query()...\t\t");
    smf_settings_set_sql_user_query(settings, TEST_SQL_QUERY);
    if (strcmp(smf_settings_get_sql_user_query(settings),TEST_SQL_QUERY) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_sql_encoding()...\t\t");
    smf_settings_set_sql_encoding(settings, TEST_ENCODING);
    if (strcmp(smf_settings_get_sql_encoding(settings),TEST_ENCODING) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_sql_max_connections()...\t");
    smf_settings_set_sql_max_connections(settings, 10);
    if (smf_settings_get_sql_max_connections(settings) != 10) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_uri()...\t\t");
    smf_settings_set_ldap_uri(settings, TEST_HOST);
    if (strcmp(smf_settings_get_ldap_uri(settings),TEST_HOST) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
#if 0
    printf("* testing smf_settings_set_ldap_host()...\t\t");
    sl = (char**)g_malloc(2*sizeof(char*));
    sl[0] = g_strdup(TEST_HOST);
    sl[1] = '\0';
    smf_settings_set_ldap_host(settings, sl);
    sl2 = smf_settings_get_ldap_host(settings);
    if (strcmp(sl[0],sl2[0]) != 0) {
        printf("\nSL: [%s] - [%s]\n",sl[0],sl2[0]);
        printf("failed\n");
        return -1;
    } else {
        if (smf_settings_get_ldap_num_hosts(settings) != 1) {
            printf("failed\n");
            return -1;
        } else {
            printf("passed\n");
        }
    }
    g_strfreev(sl);
#endif 
    printf("* testing smf_settings_set_ldap_port()...\t\t");
    smf_settings_set_ldap_port(settings, 1234);
    if (smf_settings_get_ldap_port(settings) != 1234) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_binddn()...\t\t");
    smf_settings_set_ldap_binddn(settings, TEST_USER);
    if (strcmp(smf_settings_get_ldap_binddn(settings),TEST_USER) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_bindpw()...\t\t");
    smf_settings_set_ldap_bindpw(settings, TEST_PASS);
    if (strcmp(smf_settings_get_ldap_bindpw(settings),TEST_PASS) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_base()...\t\t");
    smf_settings_set_ldap_base(settings, TEST_LDAP_BASE);
    if (strcmp(smf_settings_get_ldap_base(settings),TEST_LDAP_BASE) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_referrals()...\t\t");
    smf_settings_set_ldap_referrals(settings, 1);
    if (smf_settings_get_ldap_referrals(settings) != 1) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_scope()...\t\t");
    smf_settings_set_ldap_scope(settings, TEST_LDAP_SCOPE);
    if (strcmp(smf_settings_get_ldap_scope(settings),TEST_LDAP_SCOPE) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_set_ldap_user_query()...\t\t");
    smf_settings_set_ldap_user_query(settings, TEST_LDAP_QUERY);
    if (strcmp(smf_settings_get_ldap_user_query(settings),TEST_LDAP_QUERY) != 0) {
        printf("failed\n");
        return -1;
    } else {
        printf("passed\n");
    }
    
    printf("* testing smf_settings_group_load()...\t\t\t");
    test_group = smf_settings_group_load(settings, "global");
    if (test_group != NULL) { 
        printf("passed\n");
    } else {
        printf("failed\n");
        return -1;
    }
    
    printf("* testing smf_settings_group_get_string()...\t\t");
    s = smf_settings_group_get_string(test_group,"engine");
    if (strcmp(s,TEST_ENGINE) != 0) {
        printf("failed\n");
        if (s!=NULL)
            free(s);
        return -1;
    } else {
        printf("passed\n");
        free(s);
    }
    
    printf("* testing smf_settings_group_get_value()...\t\t");
    s = (char *)smf_settings_group_get_value(test_group,"engine");
    if (strcmp(s,TEST_ENGINE) != 0) {
        printf("failed\n");
        if (s!=NULL)
            free(s);
        return -1;
    } else {
        printf("passed\n");
        free(s);
    }

    printf("* testing smf_settings_group_get_boolean()...\t\t");
    if (smf_settings_group_get_boolean(test_group,"add_header")) {
        printf("passed\n");
    } else {
        printf("failed\n");
        return -1;
    }
    
    printf("* testing smf_settings_group_get_integer()...\t\t");
    if (smf_settings_group_get_integer(test_group,"module_fail") != 3) {
        printf("failed\n");
        return -1;
    } else
        printf("passed\n");
    
    printf("* testing smf_settings_group_get_double()...\t\t");
    if (smf_settings_group_get_double(test_group,"max_size") != 0) {
        printf("failed\n");
        return -1;
    } else
        printf("passed\n");
    
    printf("* testing smf_settings_group_get_string_list()...\t");
    sl = smf_settings_group_get_string_list(test_group,"modules",&sl_length);
    if (sl != NULL) {
        if (strcmp((char *)sl[0],"clamav") != 0) {
            printf("failed\n");
            return -1;
        }
        if (strcmp((char *)sl[1],"spamassassin") != 0) {
            printf("failed\n");
            return -1;
        }
        printf("passed\n");
        g_strfreev(sl);
    } else {
        printf("failed\n");
        return -1;
    }
    
    printf("* testing smf_settings_group_free()...\t\t\t");
    smf_settings_group_free(test_group);
    printf("passed\n");

#endif 
    
    printf("* testing smf_settings_free()...\t\t\t");
    smf_settings_free(settings);
    printf("passed\n");

    return 0;
}