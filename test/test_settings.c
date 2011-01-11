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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#define TEST_ENGINE "smtpd"
#define TEST_CONFIG_FILE "test_settings"
#define TEST_QUEUE_DIR "/tmp"
#define TEST_MODULE_1 "clamav"
#define TEST_MODULE_2 "spamassassin"
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

int main (int argc, char const *argv[]) {
	SMFSettings_T *settings = NULL;
	SMFSettingsGroup_T *test_group = NULL;
	char *s = NULL;
	char **sl = NULL;
	char **sl2 = NULL;
	int sl_length = 0;
	
	g_thread_init(NULL);
	
	if (!g_thread_supported()) {
		g_printf("glib2 does not support threads!\n");
		return -1;
	} else {
		g_printf("Start SMFSettings_T tests...\n");
		g_printf("* testing smf_settings_get()...\t\t\t\t");
		settings = smf_settings_get();		
		if (settings != NULL)
			g_printf("passed\n");
		else {
			g_printf("failed\n");
			return -1;
		}
	}
		
	g_printf("* testing smf_settings_parse_config()...\t\t");
	if (smf_settings_parse_config(settings,"../../spmfilter.conf.sample") != 0) {
		g_printf("failed\n");
		return -1;
	} else 
		g_printf("passed\n");

	g_printf("* testing smf_settings_set_debug()...\t\t\t");
	if (smf_settings_set_debug(0) != smf_settings_get_debug()) {
		g_printf("failed\n");
		return -1;
	} else {
		if (smf_settings_set_debug(3) != -1) {
			g_printf("failed\n");
			return -1;
		} else {
			g_printf("passed\n");
		}
	}
	
	g_printf("* testing smf_settings_set_config_file()...\t\t");
	if (smf_settings_set_config_file(TEST_CONFIG_FILE) != 0) {
		g_printf("1failed\n");
		return -1;
	} else {
		if (g_strcmp0(smf_settings_get_config_file(),TEST_CONFIG_FILE) != 0) {
			g_printf("failed\n");
			return -1;
		} else {
			g_printf("passed\n");
			smf_settings_set_config_file("../../spmfilter.conf.sample");
		}
	}
	
	g_printf("* testing smf_settings_set_queue_dir()...\t\t");
	if (smf_settings_set_queue_dir(TEST_QUEUE_DIR) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		if (g_strcmp0(smf_settings_get_queue_dir(),TEST_QUEUE_DIR) != 0) {
			g_printf("failed\n");
			return -1;
		} else {
			g_printf("passed\n");
		}
	}
	
	g_printf("* testing smf_settings_set_engine()...\t\t\t");
	smf_settings_set_engine(TEST_ENGINE);
	if (g_strcmp0(smf_settings_get_engine(),TEST_ENGINE) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_modules()...\t\t\t");
	sl = g_strdupv(settings->modules);
	smf_settings_set_modules(sl);
	sl2 = smf_settings_get_modules();
	if (g_strcmp0(sl[0],sl2[0]) != 0) {
		g_printf("\nSL: [%s] - [%s]\n",sl[0],sl2[0]);
		g_printf("failed\n");
		return -1;
	} else if (g_strcmp0(sl[1],sl2[1]) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	g_strfreev(sl);
	
	g_printf("* testing smf_settings_set_module_fail()...\t\t");
	smf_settings_set_module_fail(2);
	if (smf_settings_get_module_fail() == 2) {
		g_printf("passed\n");
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_set_nexthop()...\t\t\t");
	smf_settings_set_nexthop(TEST_NEXTHOP);
	if(g_strcmp0(smf_settings_get_nexthop(),TEST_NEXTHOP) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_nexthop_fail_code()...\t");
	smf_settings_set_nexthop_fail_code(450);
	if(smf_settings_get_nexthop_fail_code() != 450) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
		
	g_printf("* testing smf_settings_set_nexthop_fail_msg()...\t");
	smf_settings_set_nexthop_fail_msg(TEST_FAIL_MSG);
	if(g_strcmp0(smf_settings_get_nexthop_fail_msg(),TEST_FAIL_MSG) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}	
		
	g_printf("* testing smf_settings_set_backend()...\t\t\t");
	sl = g_strdupv(settings->backend);
	smf_settings_set_backend(sl);
	sl2 = smf_settings_get_backend();
	if (g_strcmp0(sl[0],sl2[0]) != 0) {
		g_printf("\nSL: [%s] - [%s]\n",sl[0],sl2[0]);
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	g_strfreev(sl);	
		
	g_printf("* testing smf_settings_set_backend_connection()...\t");
	smf_settings_set_backend_connection(TEST_BACKEND_CONN);
	if(g_strcmp0(smf_settings_get_backend_connection(),TEST_BACKEND_CONN) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_add_header()...\t\t");
	smf_settings_set_add_header(1);
	if(smf_settings_get_add_header() != 1) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_max_size()...\t\t");
	smf_settings_set_max_size(5000);
	if(smf_settings_get_max_size() != 5000) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_tls()...\t\t\t");
	smf_settings_set_tls(SMF_TLS_REQUIRED);
	if (smf_settings_get_tls() != SMF_TLS_REQUIRED) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_tls_pass()...\t\t");
	smf_settings_set_tls_pass(TEST_PASS);
	if(g_strcmp0(smf_settings_get_tls_pass(),TEST_PASS) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_daemon()...\t\t\t");
	smf_settings_set_daemon(1);
	if (smf_settings_get_daemon() != 1) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_driver()...\t\t");
	smf_settings_set_sql_driver(TEST_SQL_DRIVER);
	if (g_strcmp0(smf_settings_get_sql_driver(),TEST_SQL_DRIVER) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_name()...\t\t");
	smf_settings_set_sql_name(TEST_SQL_NAME);
	if (g_strcmp0(smf_settings_get_sql_name(),TEST_SQL_NAME) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_host()...\t\t");
	sl = (char**)g_malloc(2*sizeof(char*));
	sl[0] = g_strdup(TEST_HOST);
	sl[1] = NULL;
	smf_settings_set_sql_host(sl);
	sl2 = smf_settings_get_sql_host();
	if (g_strcmp0(sl[0],sl2[0]) != 0) {
		g_printf("\nSL: [%s] - [%s]\n",sl[0],sl2[0]);
		g_printf("failed\n");
		return -1;
	} else {
		if (smf_settings_get_sql_num_hosts() != 1) {
			g_printf("failed\n");
			return -1;
		} else {
			g_printf("passed\n");
		}
	}
	g_strfreev(sl);
	
	g_printf("* testing smf_settings_set_sql_port()...\t\t");
	smf_settings_set_sql_port(1234);
	if (smf_settings_get_sql_port() != 1234) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_user()...\t\t");
	smf_settings_set_sql_user(TEST_USER);
	if (g_strcmp0(smf_settings_get_sql_user(),TEST_USER) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_pass()...\t\t");
	smf_settings_set_sql_pass(TEST_PASS);
	if (g_strcmp0(smf_settings_get_sql_pass(),TEST_PASS) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_user_query()...\t\t");
	smf_settings_set_sql_user_query(TEST_SQL_QUERY);
	if (g_strcmp0(smf_settings_get_sql_user_query(),TEST_SQL_QUERY) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_encoding()...\t\t");
	smf_settings_set_sql_encoding(TEST_ENCODING);
	if (g_strcmp0(smf_settings_get_sql_encoding(),TEST_ENCODING) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_sql_max_connections()...\t");
	smf_settings_set_sql_max_connections(10);
	if (smf_settings_get_sql_max_connections() != 10) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_uri()...\t\t");
	smf_settings_set_ldap_uri(TEST_HOST);
	if (g_strcmp0(smf_settings_get_ldap_uri(),TEST_HOST) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_host()...\t\t");
	sl = (char**)g_malloc(2*sizeof(char*));
	sl[0] = g_strdup(TEST_HOST);
	sl[1] = NULL;
	smf_settings_set_ldap_host(sl);
	sl2 = smf_settings_get_ldap_host();
	if (g_strcmp0(sl[0],sl2[0]) != 0) {
		g_printf("\nSL: [%s] - [%s]\n",sl[0],sl2[0]);
		g_printf("failed\n");
		return -1;
	} else {
		if (smf_settings_get_ldap_num_hosts() != 1) {
			g_printf("failed\n");
			return -1;
		} else {
			g_printf("passed\n");
		}
	}
	g_strfreev(sl);
	
	g_printf("* testing smf_settings_set_ldap_port()...\t\t");
	smf_settings_set_ldap_port(1234);
	if (smf_settings_get_ldap_port() != 1234) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_binddn()...\t\t");
	smf_settings_set_ldap_binddn(TEST_USER);
	if (g_strcmp0(smf_settings_get_ldap_binddn(),TEST_USER) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_bindpw()...\t\t");
	smf_settings_set_ldap_bindpw(TEST_PASS);
	if (g_strcmp0(smf_settings_get_ldap_bindpw(),TEST_PASS) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_base()...\t\t");
	smf_settings_set_ldap_base(TEST_LDAP_BASE);
	if (g_strcmp0(smf_settings_get_ldap_base(),TEST_LDAP_BASE) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_referrals()...\t\t");
	smf_settings_set_ldap_referrals(1);
	if (smf_settings_get_ldap_referrals() != 1) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_scope()...\t\t");
	smf_settings_set_ldap_scope(TEST_LDAP_SCOPE);
	if (g_strcmp0(smf_settings_get_ldap_scope(),TEST_LDAP_SCOPE) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_set_ldap_user_query()...\t\t");
	smf_settings_set_ldap_user_query(TEST_LDAP_QUERY);
	if (g_strcmp0(smf_settings_get_ldap_user_query(),TEST_LDAP_QUERY) != 0) {
		g_printf("failed\n");
		return -1;
	} else {
		g_printf("passed\n");
	}
	
	g_printf("* testing smf_settings_group_load()...\t\t\t");
	test_group = smf_settings_group_load(settings, "global");
	if (test_group != NULL) { 
		g_printf("passed\n");
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_group_get_string()...\t\t");
	s = smf_settings_group_get_string(test_group,"engine");
	if (g_strcmp0(s,TEST_ENGINE) != 0) {
		g_printf("failed\n");
		if (s!=NULL)
			free(s);
		return -1;
	} else {
		g_printf("passed\n");
		free(s);
	}
	
	g_printf("* testing smf_settings_group_get_value()...\t\t");
	s = (char *)smf_settings_group_get_value(test_group,"engine");
	if (g_strcmp0(s,TEST_ENGINE) != 0) {
		g_printf("failed\n");
		if (s!=NULL)
			free(s);
		return -1;
	} else {
		g_printf("passed\n");
		free(s);
	}
	
	g_printf("* testing smf_settings_group_get_boolean()...\t\t");
	if (smf_settings_group_get_boolean(test_group,"add_header")) {
		g_printf("passed\n");
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_group_get_integer()...\t\t");
	if (smf_settings_group_get_integer(test_group,"module_fail") != 3) {
		g_printf("failed\n");
		return -1;
	} else
		g_printf("passed\n");
	
	g_printf("* testing smf_settings_group_get_double()...\t\t");
	if (smf_settings_group_get_double(test_group,"max_size") != 0) {
		g_printf("failed\n");
		return -1;
	} else
		g_printf("passed\n");
	
	g_printf("* testing smf_settings_group_get_string_list()...\t");
	sl = smf_settings_group_get_string_list(test_group,"modules",&sl_length);
	if (sl != NULL) {
		if (g_strcmp0((char *)sl[0],"clamav") != 0) {
			g_printf("failed\n");
			return -1;
		}
		if (g_strcmp0((char *)sl[1],"spamassassin") != 0) {
			g_printf("failed\n");
			return -1;
		}
		g_printf("passed\n");
		g_strfreev(sl);
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_group_free()...\t\t\t");
	smf_settings_group_free(test_group);
	g_printf("passed\n");
	
	
	g_printf("* testing smf_settings_free()...\t\t\t");
	smf_settings_free(settings);
	g_printf("passed\n");
	
	g_thread_exit(NULL);
	return 0;
}