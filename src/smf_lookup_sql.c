/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner, Werner Detter and SpaceNet AG
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

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#include <ConnectionPool.h>
#include <SQLException.h>
#include <Exception.h>

#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_core.h"
#include "smf_dict.h"
#include "smf_list.h"

#define THIS_MODULE "sql_lookup"

#define FIELDSIZE 1024

ConnectionPool_T sql_pool = NULL;
URL_T url = NULL;

void sql_abort_handler(const char *error);
int sql_start_pool(SMFSettings_T *settings, char *dsn);

void _sql_result_list_destroy(void *data) {
    assert(data);
    smf_dict_free((SMFDict_T *)data);
}

/** Disconnect from sql server and destroy connection pool */
void smf_lookup_sql_disconnect(void) {
	TRACE(TRACE_LOOKUP,"closing database connection");
	ConnectionPool_stop(sql_pool);
	ConnectionPool_free(&sql_pool);
	URL_free(&url);
}

/** Return connection to connection pool
 *
 * \param c connection to close
 */
void sql_con_close(Connection_T c) {
	TRACE(TRACE_LOOKUP,"returning connection to pool");
	Connection_close(c);
	return;
}

/** Get random sql host
 *
 * \returns hostname of sql server
 */
char *sql_get_rand_host(SMFSettings_T *settings) {
 assert(settings);
	int random;
	SMFListElem_T *e = NULL;
	int count = 0;

	TRACE(TRACE_DEBUG,"trying to get random sql server");
	srand(time(NULL));
	random = rand() % smf_list_size(settings->sql_host);
	e = smf_list_head(settings->sql_host);

	while(e != NULL) {     
 	count++;
  if(count != random) {
  		printf("chosen[%s]", (char *)smf_list_data(e));
  		return (char *)smf_list_data(e);
  }
		e = e->next;
	}

	return NULL;
}


char *sql_get_dsn(SMFSettings_T *settings, char *host) {
	assert(settings);

	GString *sdsn = g_string_new("");
	char *dsn;
	
	if (settings->sql_driver != NULL) {
		g_string_append_printf(sdsn,"%s://",settings->sql_driver);
	} else {
		TRACE(TRACE_ERR,"error, no sql driver defined!");
		return NULL;
	}

	if (host != NULL)
		g_string_append_printf(sdsn, "%s", host);
	
	if (settings->sql_port)
		g_string_append_printf(sdsn, ":%u", settings->sql_port);

	if (settings->sql_name) {
		if (g_ascii_strcasecmp(settings->sql_driver,"sqlite") == 0) {
			/* expand ~ in db name to HOME env variable */
			if ((strlen(settings->sql_name) > 0 ) && (settings->sql_name[0] == '~')) {
				char *homedir;
				char *db = NULL;
				if ((homedir = getenv ("HOME")) == NULL)
					TRACE(TRACE_ERR,"can't expand ~ in db name");
				g_snprintf(db, FIELDSIZE, "%s%s", homedir, &(settings->sql_name[1]));
				g_strlcpy(settings->sql_name, db, FIELDSIZE);
				g_free(db);
			}
			g_string_append_printf(sdsn, "%s", settings->sql_name);
		} else {
			g_string_append_printf(sdsn,"/%s",settings->sql_name);
		}
	}

	if (settings->sql_user && strlen((const char*)settings->sql_user)) {
		g_string_append_printf(sdsn,"?user=%s", settings->sql_user);
		if (settings->sql_pass && strlen((const char *)settings->sql_pass))
			g_string_append_printf(sdsn,"&password=%s", settings->sql_pass);
		if (g_ascii_strcasecmp(settings->sql_driver,"mysql") == 0) {
			if (settings->sql_encoding && strlen((const char *)settings->sql_encoding))
				g_string_append_printf(sdsn,"&charset=%s", settings->sql_encoding);
		}
	}

	TRACE(TRACE_LOOKUP,"sql db at url: [%s]", sdsn->str);
	dsn = g_strdup(sdsn->str);
	g_string_free(sdsn,TRUE);	
	return dsn;
}


/** Try to start a new connection pool
 *
 * \param dsn for new server connection
 *
 * \returns 0 on success or -1 in case of error
 */
int sql_start_pool(SMFSettings_T *settings, char *dsn) {
	assert(settings);

	int sweep_interval = 60;
	Connection_T con = NULL;

	url = URL_new(dsn);
	g_free(dsn);

	if (!(sql_pool = ConnectionPool_new(url))) {
		TRACE(TRACE_ERR,"error creating database connection pool");
		return -1;
	}

	if (settings->sql_max_connections > 0) {
		if (settings->sql_max_connections < (unsigned int)ConnectionPool_getInitialConnections(sql_pool))
			ConnectionPool_setInitialConnections(sql_pool, settings->sql_max_connections);
		ConnectionPool_setMaxConnections(sql_pool, settings->sql_max_connections);
		TRACE(TRACE_LOOKUP,"database connection pool created with maximum connections of [%d]",settings->sql_max_connections);
	}

	ConnectionPool_setReaper(sql_pool, sweep_interval);

	if (g_ascii_strcasecmp(settings->sql_driver,"sqlite") != 0) 
		ConnectionPool_setAbortHandler(sql_pool, sql_abort_handler);

	TRACE(TRACE_LOOKUP, "run a database connection reaper thread every [%d] seconds", sweep_interval);

	ConnectionPool_start(sql_pool);

	TRACE(TRACE_LOOKUP, "database connection pool started with [%d] connections, max [%d]",
			ConnectionPool_getInitialConnections(sql_pool), ConnectionPool_getMaxConnections(sql_pool));

	if (!(con = ConnectionPool_getConnection(sql_pool))) {
		TRACE(TRACE_ERR, "error getting a database connection from the pool");

		return -1;
	}
	sql_con_close(con);

	return 0;
}


void sql_abort_handler(const char *error) {
	TRACE(TRACE_ERR, "%s", error);	
}


int smf_lookup_sql_connect(SMFSettings_T *settings) {
	assert(settings);

	char *dsn = NULL;
	/* try to get a random host if backend_connection is set to "balance"
	 * and the database driver is not sqlite */
	if ((g_ascii_strcasecmp(settings->backend_connection,"balance") == 0) &&
			(g_ascii_strcasecmp(settings->sql_driver,"sqlite") != 0)) {
			dsn = sql_get_dsn(settings, sql_get_rand_host(settings));
	} else {
		if (g_ascii_strcasecmp(settings->sql_driver,"sqlite") != 0) {
			SMFListElem_T *e = NULL;
			e = smf_list_head(settings->sql_host);
			dsn = sql_get_dsn(settings, (char *)smf_list_data(e));
			smf_settings_set_active_lookup_host(settings, (char *)smf_list_data(e));
		} else {
			dsn = sql_get_dsn(settings, NULL);
		}
	}
	if(sql_start_pool(settings,dsn) != 0)
		return -1;
	else
		return 0;
}

int smf_lookup_sql_connect_fallback(SMFSettings_T *settings) {
	assert(settings);

	SMFList_T *l = NULL;
	SMFListElem_T *e = NULL;
	char *dsn=NULL;

	if (g_ascii_strcasecmp(settings->sql_driver,"sqlite") != 0) {
		l = smf_settings_get_sql_hosts(settings);
		e = smf_list_head(l);

		while(e != NULL) {
			if(strcasecmp(smf_settings_get_active_lookup_host(settings), 
			(char *)smf_list_data(e)) != 0) {
					dsn = sql_get_dsn(settings, (char *)smf_list_data(e));
					smf_settings_set_active_lookup_host(settings, (char *)smf_list_data(e));
			}
			e = e->next;
		}
	}
	smf_lookup_sql_disconnect();
	if(sql_start_pool(settings,dsn) != 0)
		return -1;
	else
		return 0;
}

/** Get open connection from connection pool
 *
 * \returns connection
 */
Connection_T sql_con_get(void) {
	int i=0, k=0; 
	Connection_T c;
	while (i++<30) {
		c = ConnectionPool_getConnection(sql_pool);
		if (c) break;
		if((int)(i % 5)==0) {
			TRACE(TRACE_WARNING, "Thread is having trouble obtaining a database connection. Try [%d]", i);
			k = ConnectionPool_reapConnections(sql_pool);
			TRACE(TRACE_LOOKUP, "Database reaper closed [%d] stale connections", k);
		}
		sleep(1);
	}
	if (! c) {
		TRACE(TRACE_ERR,"[%p] can't get a database connection from the pool! max [%d] size [%d] active [%d]", 
			sql_pool,
			ConnectionPool_getMaxConnections(sql_pool),
			ConnectionPool_size(sql_pool),
			ConnectionPool_active(sql_pool));
	}

	assert(c);
	TRACE(TRACE_LOOKUP,"[%p] connection from pool", c);
	return c;
}



SMFList_T *smf_lookup_sql_query(SMFSettings_T *settings, const char *q, ...) {	
	Connection_T c; 
	ResultSet_T r;
	SMFList_T *result;
	va_list ap, cp;
	char *query;
	int i;

	va_start(ap, q);
	va_copy(cp, ap);
	query = (char *)malloc(strlen(q) + 1);
	vsprintf(query,q,cp);
	va_end(cp);
	smf_core_strstrip(query);

	if (strlen(query) == 0)
		return NULL;

	if (smf_list_new(&result,_sql_result_list_destroy)!=0) {
 	return NULL;
 } else {
 	c = sql_con_get();
		TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
		
		if(Connection_ping(c) == 0) {
			smf_lookup_sql_connect_fallback(settings);
			c = sql_con_get();
		}

		TRY
			r = Connection_executeQuery(c, query,NULL);
		CATCH(SQLException)
			TRACE(TRACE_ERR,"got SQLException");
			return NULL;
		END_TRY;
		
		while (ResultSet_next(r)) {
			SMFDict_T *d = smf_dict_new();

			for (i=1; i <= ResultSet_getColumnCount(r); i++) {
				int blob_size = 0;
				char *c = (char *)ResultSet_getColumnName(r,i);	
				char *col_name = NULL;
				col_name = strdup(c);
				int col_size = ResultSet_getColumnSize(r,i);
				const void *data = ResultSet_getBlob(r, i, &blob_size);

				smf_dict_set(d,col_name,data);
				free(col_name);
			}
			
			if (smf_list_append(result,d) != 0)
								return NULL;
		}

		TRACE(TRACE_LOOKUP,"[%p] found [%d] rows", c, result->size);
 }

 free(query);
 sql_con_close(c);
 return result;
}

/** Check if given user exists in database
 *
 * \param user a SMFEmailAddress_T object
 */
void smf_lookup_sql_check_user(SMFSettings_T *settings, SMFEmailAddress_T *user) {
//	SMFSettings_T *settings = smf_settings_get();
char *query;

	smf_core_expand_string(settings->sql_user_query,user->email,&query);
//	user->user_data = NULL;
//	user->user_data = smf_lookup_sql_query(query);
free(query);
}

