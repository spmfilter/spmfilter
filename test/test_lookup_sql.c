/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Werner Detter and SpaceNet AG
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
#include <db.h>
#include <glib/gprintf.h>
#include <assert.h>
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_dict.h"
#include "../src/smf_internal.h"


/*
crate database test_lookup_sql_table
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


int main (int argc, char const *argv[]) {
	int i;
	char *sql_driver = SQL_DRIVER;
	char *sql_user = SQL_USER;
	char *sql_pass = SQL_PASS;
	char *sql_query = SQL_QUERY;
	char *sql_name = SQL_NAME;
	char *sql_backend_conn = SQL_BACKEND_CONN;
	char *host1 = strdup(SQL_HOST1);
	char *host2 = strdup(SQL_HOST2);
   
	SMFList_T *result = NULL;
	SMFListElem_T *e = NULL;
	SMFSettings_T *settings = smf_settings_new();
	SMFDict_T *d = NULL;

	smf_settings_add_sql_host(settings, host1);
	smf_settings_add_sql_host(settings, host2);
	smf_settings_set_sql_driver(settings, sql_driver);
	smf_settings_set_sql_port(settings, SQL_PORT);
	smf_settings_set_sql_user(settings, sql_user);
	smf_settings_set_sql_pass(settings, sql_pass);
	smf_settings_set_sql_name(settings, sql_name);
	smf_settings_set_sql_max_connections(settings, 5);
	smf_settings_set_sql_user_query(settings, sql_query);
	smf_settings_set_backend_connection(settings, sql_backend_conn);

	if(smf_lookup_sql_connect(settings) == 0) {
		result = smf_lookup_sql_query(settings,sql_query);

		e = smf_list_head(result);
		while(e != NULL) {
			d = (SMFDict_T *)smf_list_data(e);
			printf("[%s]",smf_dict_get(d,"data"));
			e = e->next;
		}

		smf_list_free(result);
		smf_lookup_sql_disconnect();
	} else {
		printf("unable to establish database connection\n");
	}
	
	smf_settings_free(settings);

	return 0;
}

