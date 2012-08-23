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

int main (int argc, char const *argv[]) {
    SMFSettings_T *settings = NULL;
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


    printf("* testing smf_settings_set_sql_port()...\t\t");
    smf_settings_set_sql_port(settings, 1234);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_port()...\t\t");
    if (smf_settings_get_sql_port(settings) != 1234) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
        
    printf("* testing smf_settings_set_sql_user()...\t\t");
    smf_settings_set_sql_user(settings, test_email);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_user()...\t\t");
    if (strcmp(smf_settings_get_sql_user(settings),test_email) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_sql_pass()...\t\t");
    smf_settings_set_sql_pass(settings, test_string);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_pass()...\t\t");
    if (strcmp(smf_settings_get_sql_pass(settings),test_string) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_sql_user_query()...\t\t");
    smf_settings_set_sql_user_query(settings, test_sql_query);
    printf("passed\n");
    
    printf("* testing smf_settings_get_sql_user_query()...\t\t");
    if (strcmp(smf_settings_get_sql_user_query(settings),test_sql_query) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_sql_encoding()...\t\t");
    smf_settings_set_sql_encoding(settings, test_encoding);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_encoding()...\t\t");
    if (strcmp(smf_settings_get_sql_encoding(settings),test_encoding) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_sql_max_connections()...\t");
    smf_settings_set_sql_max_connections(settings, 10);
    printf("passed\n");

    printf("* testing smf_settings_get_sql_max_connections()...\t");
    if (smf_settings_get_sql_max_connections(settings) != 10) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    
    printf("* testing smf_settings_set_ldap_uri()...\t\t");
    smf_settings_set_ldap_uri(settings, test_ip);
    printf("passed\n");
    
    printf("* testing smf_settings_get_ldap_uri()...\t\t");
    if (strcmp(smf_settings_get_ldap_uri(settings),test_ip) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_add_ldap_host()...\t\t");
    s = strdup(test_ip);
    smf_settings_add_ldap_host(settings, s);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_hosts()...\t\t");
    list = smf_settings_get_ldap_hosts(settings);
    if (smf_list_size(list)!=1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_settings_set_ldap_port()...\t\t");
    smf_settings_set_ldap_port(settings, 1234);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_port()...\t\t");
    if (smf_settings_get_ldap_port(settings) != 1234) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_ldap_binddn()...\t\t");
    smf_settings_set_ldap_binddn(settings, test_email);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_binddn()...\t\t");
    if (strcmp(smf_settings_get_ldap_binddn(settings),test_email) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_ldap_bindpw()...\t\t");
    smf_settings_set_ldap_bindpw(settings, test_string);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_bindpw()...\t\t");
    if (strcmp(smf_settings_get_ldap_bindpw(settings),test_string) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_ldap_base()...\t\t");
    smf_settings_set_ldap_base(settings, test_ldap_base);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_base()...\t\t");
    if (strcmp(smf_settings_get_ldap_base(settings),test_ldap_base) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_settings_set_ldap_referrals()...\t\t");
    smf_settings_set_ldap_referrals(settings, 1);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_referrals()...\t\t");
    if (smf_settings_get_ldap_referrals(settings) != 1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_ldap_scope()...\t\t");
    smf_settings_set_ldap_scope(settings, test_ldap_scope);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_scope()...\t\t");
    if (strcmp(smf_settings_get_ldap_scope(settings),test_ldap_scope) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_settings_set_ldap_user_query()...\t\t");
    smf_settings_set_ldap_user_query(settings, test_ldap_query);
    printf("passed\n");

    printf("* testing smf_settings_get_ldap_user_query()...\t\t");
    if (strcmp(smf_settings_get_ldap_user_query(settings),test_ldap_query) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_settings_set_active_lookup_host()...\t");
    smf_settings_set_active_lookup_host(settings, test_ip);
    printf("passed\n");

    printf("* testing smf_settings_get_active_lookup_host()...\t");
    if (strcmp(smf_settings_get_active_lookup_host(settings),test_ip) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_settings_free()...\t\t\t");
    smf_settings_free(settings);
    printf("passed\n");

    return 0;
}