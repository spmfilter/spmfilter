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
#include <glib.h>
#include <db.h>
#include <glib/gprintf.h>
#include <assert.h>
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"


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


#define SQL_HOST1   "localhost"
#define SQL_HOST2   "localhost"
#define SQL_DRIVER  "mysql"
#define SQL_USER    "test_lookup_sql"
#define SQL_PASS    "MndDksjs"
#define SQL_NAME    "test_lookup_sql"
#define SQL_PORT    3306
#define SQL_BACKEND_CONN "failover"
#define SQL_QUERY   "SELECT data FROM test_lookup_sql_table LIMIT 1"
#define SQL_QUERY_RESULT_STRING "LoremIpsumDolorSitAmet"


// grant all on test_lookup_sql.* to 'test_lookup_sql'@'localhost' IDENTIFIED BY 'MndDksjs';  

int main (int argc, char const *argv[]) {
    int i;
    char *sql_driver = SQL_DRIVER;
    char *sql_user = SQL_USER;
    char *sql_pass = SQL_PASS;
    char *sql_query = SQL_QUERY;
    char *sql_name = SQL_NAME;
    char *sql_backend_conn = SQL_BACKEND_CONN;
    char **sql_host = (char**)g_malloc(3*sizeof(char*));

    sql_host[0] = g_strdup(SQL_HOST1);
    sql_host[1] = g_strdup(SQL_HOST2);
    sql_host[2] = '\0';

    
    SMFLookupResult_T *result = NULL;
    SMFSettings_T *settings = smf_settings_new();

    smf_settings_set_sql_host(settings, sql_host);
    smf_settings_set_sql_driver(settings, sql_driver);
    smf_settings_set_sql_port(settings, SQL_PORT);
    smf_settings_set_sql_user(settings, sql_user);
    smf_settings_set_sql_pass(settings, sql_pass);
    smf_settings_set_sql_name(settings, sql_name);
    smf_settings_set_sql_max_connections(settings, 5);
    smf_settings_set_sql_user_query(settings, sql_query);
    smf_settings_set_backend_connection(settings, sql_backend_conn);
    smf_lookup_sql_connect(settings);

    result = smf_lookup_sql_query(sql_query);
    
    void *bla = NULL;

    if(result != NULL) {
        if(result->len > 0) {
            for(i=0; i < result->len; i++) {
                 SMFLookupElement_T *elem = smf_lookup_result_index(result,i);
                 //data = (void *) smf_lookup_element_get(elem, "data");
                 bla = (char *) smf_lookup_element_get(elem, "data");

                 printf("RES:[%p], RES[%s]\n", bla, bla);
                 //assert(strcmp((char *)data,SQL_QUERY_RESULT_STRING)==0);
            }
        }
    }
    
    
    smf_lookup_result_free(result);
    smf_lookup_sql_disconnect();
    smf_settings_free(settings);

    if(sql_host[0] != NULL)
        free(sql_host[0]);

    if(sql_host[1] != NULL)
        free(sql_host[1]);

    if(sql_host != NULL)
        g_free(sql_host);

    if(bla != NULL)
        free(bla);

    return 0;
}

