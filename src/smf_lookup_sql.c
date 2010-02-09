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

#include "spmfilter.h"
#include "smf_lookup.h"

#define THIS_MODULE "sql_lookup"

#define FIELDSIZE 1024

ConnectionPool_T sql_pool = NULL;
URL_T url = NULL;

static void sql_fallback_handler(const char *error);
int sql_start_pool(char *dsn);

int active_server = -1;

/** Disconnect from sql server and destroy connection pool */
void sql_disconnect(void) {
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
char *sql_get_rand_host(void) {
	Settings_T *settings = smf_settings_get();
	TRACE(TRACE_DEBUG,"trying to get random sql server");
	srand(time(NULL));
	return settings->sql_host[rand() % settings->sql_num_hosts];
}

/** Build a DSN string
 *
 * \param host to connect
 *
 * \returns dsn string
 */
char *sql_get_dsn(char *host) {
	GString *sdsn = g_string_new("");
	char *dsn;
	Settings_T *settings = smf_settings_get();
	
	if (settings->sql_driver != NULL) {
		g_string_append_printf(sdsn,"%s://",settings->sql_driver);
	} else {
		TRACE(TRACE_ERR,"arning, no sql driver defined!");
		return NULL;
	}

	g_string_append_printf(sdsn, "%s", host);
	
	if (settings->sql_port)
		g_string_append_printf(sdsn, "%u", settings->sql_port);

	if (settings->sql_name) {
		if (g_ascii_strcasecmp(settings->sql_driver,"sqlite") == 0) {
			/* expand ~ in db name to HOME env variable */
			if ((strlen(settings->sql_name) > 0 ) && (settings->sql_name[0] == '~')) {
				char *homedir;
				char *db;
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
int sql_start_pool(char *dsn) {
	Settings_T *settings = smf_settings_get();
	int sweep_interval = 60;
	Connection_T con = NULL;


	url = URL_new(dsn);

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
		ConnectionPool_setAbortHandler(sql_pool, sql_fallback_handler);

	TRACE(TRACE_LOOKUP, "run a database connection reaper thread every [%d] seconds", sweep_interval);

	ConnectionPool_start(sql_pool);

	TRACE(TRACE_LOOKUP, "database connection pool started with [%d] connections, max [%d]",
			ConnectionPool_getInitialConnections(sql_pool), ConnectionPool_getMaxConnections(sql_pool));

	if (!(con = ConnectionPool_getConnection(sql_pool))) {
		sql_con_close(con);
		TRACE(TRACE_ERR, "error getting a database connection from the pool");
		return -1;
	}
	sql_con_close(con);

	return 0;
}

/** Fallback function if connection dies or server is not available.
 *  If more than one server is configured, try to establish a connection
 *  to one of the remaining server.
 *
 * \param error exception message
 */
static void sql_fallback_handler(const char *error) {
	Settings_T *settings = smf_settings_get();
	char *dsn;
	TRACE(TRACE_ERR, "%s", error);

	if (active_server == -1)
		active_server = 0;
	else if (active_server < (settings->sql_num_hosts - 1))
		active_server++;
	else {
		TRACE(TRACE_CRIT,"no sql server available");
		exit(1);
	}
	
	TRACE(TRACE_WARNING,"trying sql failover connection to [%s]", settings->sql_host[active_server]);
	dsn = sql_get_dsn(settings->sql_host[active_server]);
	sql_disconnect();
	sql_start_pool(dsn);
}

/** Connect to sql server
 *
 * \returns 0 on success or -1 in case of error
 */
int sql_connect(void) {
	
	Settings_T *settings = smf_settings_get();
	char *dsn = NULL;

	/* try to get a random host if backend_connection is set to "balance"
	 * and the database driver is not sqlite */
	if ((g_ascii_strcasecmp(settings->backend_connection,"balance") == 0) &&
			(g_ascii_strcasecmp(settings->sql_driver,"sqlite") != 0))
		dsn = sql_get_dsn(sql_get_rand_host());
	else {
		dsn = sql_get_dsn(settings->sql_host[0]);
		active_server = 0;
	}

	if(sql_start_pool(dsn) != 0)
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


LookupResult_T *sql_query(const char *q, ...) {
	Connection_T c; 
	ResultSet_T r;
	LookupResult_T *result = lookup_result_new();
	va_list ap, cp;
	char *query;
	int i;

	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
		
	c = sql_con_get();
	TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
	TRY
		r = Connection_executeQuery(c, query,NULL);
	CATCH(SQLException)
		TRACE(TRACE_ERR,"got SQLException");
		return NULL;
	END_TRY;

	while (ResultSet_next(r)) {
		LookupElement_T *e = lookup_element_new();
			
		for (i=1; i <= ResultSet_getColumnCount(r); i++) {
			int blob_size = 0;
			char *c = (char *)ResultSet_getColumnName(r,i);
			char *col_name = (char *)malloc(strlen(c) + 1);
			memcpy(col_name,c,sizeof(c));
			int col_size = ResultSet_getColumnSize(r,i);
			void *data = malloc(col_size);
			memcpy(data,ResultSet_getBlob(r,i,&blob_size),col_size);
			lookup_element_add(e,col_name,data);
		}
		lookup_result_add(result,e);
	}
	TRACE(TRACE_LOOKUP,"[%p] found [%d] rows", c, result->len);
	return result;
}


/** check if given user exists in database
 *
 * \param addr email adress of user
 *
 * \return 1 if the user exists, otherwise 0
 */
int sql_user_exists(char *addr) {
	Connection_T c = NULL;
	ResultSet_T r = NULL;
	char *query = NULL;
	Settings_T *settings = smf_settings_get();

	c = sql_con_get();
	if (expand_query(settings->sql_user_query, addr, &query) <= 0) {
		TRACE(TRACE_ERR,"failed to expand sql query");
		return -1;
	}
	TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
	r = Connection_executeQuery(c,query,NULL);
	if (query != NULL)
		free(query);
	if (ResultSet_next(r)) {
		TRACE(TRACE_LOOKUP, "found user [%s]",addr);
		sql_con_close(c);
		return 1;
	} else {
		TRACE(TRACE_LOOKUP, "user [%s] does not exist", addr);
		sql_con_close(c);
		return 0;
	}
}

