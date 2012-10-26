/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Werner Detter and SpaceNet AG
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_sql.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_dict.h"
#include "../src/smf_internal.h"

/*
create database test_lookup_sql
grant all on test_lookup_sql.* to 'test_lookup_sql'@'localhost' IDENTIFIED BY 'MndDksjs'; 

DROP TABLE IF EXISTS `test_lookup_sql_table`;
CREATE TABLE `test_lookup_sql_table` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `data` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;

INSERT INTO `test_lookup_sql_table` VALUES (1,'LoremIpsumDolorSitAmet');
*/

#define SQL_HOST1   "127.0.0.1"
#define SQL_HOST2   "localhost"
#define SQL_DRIVER  "mysql"
#define SQL_USER    "test_lookup_sql"
#define SQL_PASS    "MndDksjs"
#define SQL_NAME    "test_lookup_sql"
#define SQL_PORT    3306
#define SQL_BACKEND_CONN "failover"
#define SQL_QUERY   "SELECT data FROM test_lookup_sql_table LIMIT 1"
#define SQL_QUERY_RESULT_STRING "LoremIpsumDolorSitAmet"


int test_query(SMFSettings_T *settings) {
    SMFList_T *result = NULL;
    SMFListElem_T *e = NULL;
    SMFDict_T *d = NULL;
    int found = 0;

    result = smf_lookup_sql_query(settings,SQL_QUERY);
    e = smf_list_head(result);
    while(e != NULL) {
        d = (SMFDict_T *)smf_list_data(e);
        if (strcmp(smf_dict_get(d,"data"),SQL_QUERY_RESULT_STRING)==0) {
            found = 1;
            break;
        }
        e = e->next;
    }
    smf_list_free(result);

    return found;
}

int main (int argc, char const *argv[]) {
	char *host1 = strdup(SQL_HOST1);
	char *host2 = strdup(SQL_HOST2);
   
	SMFSettings_T *settings = smf_settings_new();
	

	printf("Start smf_lookup_sql tests...\n");
    printf("==================================================\n");
    printf("This tests expects a running MySQL Server at localhost\n\
with following scheme:\n\n\
create database test_lookup_sql;\n\
grant all on test_lookup_sql.* to 'test_lookup_sql'@'localhost' IDENTIFIED BY 'MndDksjs'; \n\
\n\
DROP TABLE IF EXISTS `test_lookup_sql_table`;\n\
CREATE TABLE `test_lookup_sql_table` (\n\
  `id` int(11) NOT NULL AUTO_INCREMENT,\n\
  `data` varchar(100) DEFAULT NULL,\n\
  PRIMARY KEY (`id`)\n\
) ENGINE=MyISAM AUTO_INCREMENT=2 DEFAULT CHARSET=latin1;\n\
\n\
INSERT INTO `test_lookup_sql_table` VALUES (1,'LoremIpsumDolorSitAmet');\n\
==================================================\n");

	smf_settings_add_sql_host(settings, host1);
	smf_settings_add_sql_host(settings, host2);
	smf_settings_set_sql_driver(settings, SQL_DRIVER);
	smf_settings_set_sql_port(settings, SQL_PORT);
	smf_settings_set_sql_user(settings, SQL_USER);
	smf_settings_set_sql_pass(settings, SQL_PASS);
	smf_settings_set_sql_name(settings, SQL_NAME);
	smf_settings_set_sql_max_connections(settings, 5);
	smf_settings_set_sql_user_query(settings, SQL_QUERY);
	smf_settings_set_backend_connection(settings, SQL_BACKEND_CONN);
	smf_settings_set_debug(settings,1);
    smf_settings_set_lookup_persistent(settings, 1);

	printf("* testing smf_lookup_sql_connect()...\t\t\t\t");
    if(smf_lookup_sql_connect(settings) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_lookup_sql_query()...\t\t\t\t");
    if (test_query(settings)==1) { 
        printf("passed\n");
    } else {
        printf("failed\b");
        return -1;
    }

    printf("* testing smf_lookup_sql_disconnect()...\t\t\t");
    smf_lookup_sql_disconnect(settings);
    printf("passed\n");

    smf_settings_set_lookup_persistent(settings, 0);

    printf("* testing smf_lookup_sql_query() without existing connection...\t");
    if (test_query(settings)==1) { 
        printf("passed\n");
    } else {
        printf("failed\b");
        return -1;
    }

	smf_settings_free(settings);

	return 0;
}

