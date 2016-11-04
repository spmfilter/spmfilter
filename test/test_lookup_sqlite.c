/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2016 Axel Steiner, Werner Detter and SpaceNet AG
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

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "test.h"
#include "test_params.h"
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_sql.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_session.h"
#include "../src/smf_dict.h"
#include "../src/smf_internal.h"

#define SQL_DRIVER  "sqlite"
#define SQL_SQLITE_DB "test_lookup_sqlite.db"
#define SQL_QUERY   "select data from test_lookup_sqlite"
#define SQL_QUERY_RESULT_STRING "LoremIpsumDolorSitAmet"
#define SQL_BACKEND_CONN "failover"


int main (int argc, char const *argv[]) {
	char *sql_driver = SQL_DRIVER;
	char *sql_query = SQL_QUERY;
	char *sql_name = NULL;
	char *sql_backend_conn = SQL_BACKEND_CONN;
   
	SMFList_T *result = NULL;
	SMFListElem_T *e = NULL;
	SMFSettings_T *settings = smf_settings_new();
	SMFSession_T *session = smf_session_new();
	SMFDict_T *d = NULL;
	int found = 0;


	printf("* testing smf_lookup_sql_connect()...\t\t\t\t");
	asprintf(&sql_name, "%s/%s", SAMPLES_DIR, SQL_SQLITE_DB);
	smf_settings_set_sql_driver(settings, sql_driver);
	smf_settings_set_sql_name(settings, sql_name);
	smf_settings_set_backend_connection(settings, sql_backend_conn);
	smf_settings_set_sql_user_query(settings, sql_query);
	smf_settings_set_sql_max_connections(settings, 5);

	if(smf_lookup_sql_connect(settings) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

	printf("* testing smf_lookup_sql_query()...\t\t\t\t");
	result = smf_lookup_sql_query(settings,session,sql_query);
		
	e = smf_list_head(result);
	while(e != NULL) {
		d = (SMFDict_T *)smf_list_data(e);
		if (strcmp(smf_dict_get(d,"data"),SQL_QUERY_RESULT_STRING)==0)
			found = 1;
		e = e->next;
	}

	if (found!=1) {
		printf("failed\n");
		return -1;
	}
	printf("passed\n");
	smf_list_free(result);
	

	printf("* testing smf_lookup_sql_disconnect()...\t\t\t");
    smf_lookup_sql_disconnect(settings);
    printf("passed\n");

	if(sql_name != NULL)
		free(sql_name);

	smf_session_free(session);
	smf_settings_free(settings);

	return 0;
}
