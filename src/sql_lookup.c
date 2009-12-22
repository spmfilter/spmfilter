#include "config.h"

#ifdef HAVE_ZDB
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#include <ConnectionPool.h>
#include <SQLException.h>

#include "spmfilter.h"

#define THIS_MODULE "sql_lookup"

ConnectionPool_T sql_pool = NULL;

void sql_con_close(Connection_T c) {
	TRACE(TRACE_LOOKUP,"closing pool connection");
	Connection_close(c);
	return;
}

int sql_connect(void) {
	URL_T url = NULL;
	Connection_T con = NULL;
	GString *dsn = g_string_new("");
	int sweep_interval = 60;
//	SETTINGS *settings = g_private_get(settings_key);

	if (settings->sql_driver != NULL) {
		g_string_append_printf(dsn,"%s://",settings->sql_driver);
	} else {
		TRACE(TRACE_ERR,"arning, no sql driver defined!");
		return -1;
	}
	
	if (settings->sql_host) 
		g_string_append_printf(dsn, "%s", settings->sql_host);
		
	if (settings->sql_port)
		g_string_append_printf(dsn, "%u", settings->sql_port);
	
	if (settings->sql_name) {
		if (MATCH(settings->sql_driver,"sqlite")) {
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
			g_string_append_printf(dsn, "%s", settings->sql_name);
		} else {
			g_string_append_printf(dsn,"/%s",settings->sql_name);
		}
	}
	
	if (settings->sql_user && strlen((const char*)settings->sql_user)) {
		g_string_append_printf(dsn,"?user=%s", settings->sql_user);
		if (settings->sql_pass && strlen((const char *)settings->sql_pass))
			g_string_append_printf(dsn,"&password=%s", settings->sql_pass);
		if (MATCH(settings->sql_driver,"mysql")) {
			if (settings->sql_encoding && strlen((const char *)settings->sql_encoding))
				g_string_append_printf(dsn,"&charset=%s", settings->sql_encoding);
		}
	}
	
	TRACE(TRACE_LOOKUP,"sql db at url: [%s]", dsn->str);	

	url = URL_new(dsn->str);
	g_string_free(dsn,TRUE);
	
	if (! (sql_pool = ConnectionPool_new(url)))
		TRACE(TRACE_ERR,"error creating database connection pool");
	
	if (settings->sql_max_connections > 0) {
		if (settings->sql_max_connections < (unsigned int)ConnectionPool_getInitialConnections(sql_pool))
			ConnectionPool_setInitialConnections(sql_pool, settings->sql_max_connections);
		ConnectionPool_setMaxConnections(sql_pool, settings->sql_max_connections);
		TRACE(TRACE_LOOKUP,"database connection pool created with maximum connections of [%d]",settings->sql_max_connections);
	}

	ConnectionPool_setReaper(sql_pool, sweep_interval);
	TRACE(TRACE_LOOKUP, "run a database connection reaper thread every [%d] seconds", sweep_interval);

	ConnectionPool_start(sql_pool);
	TRACE(TRACE_LOOKUP, "database connection pool started with [%d] connections, max [%d]", 
			ConnectionPool_getInitialConnections(sql_pool), ConnectionPool_getMaxConnections(sql_pool));

	if (! (con = ConnectionPool_getConnection(sql_pool))) {
		sql_con_close(con);
		TRACE(TRACE_ERR, "error getting a database connection from the pool");
		return -1;
	}

	sql_con_close(con);

	return 0;
}

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


ResultSet_T sql_query(const char *q, ...) {
	Connection_T c; 
	ResultSet_T r;
	va_list ap, cp;
	char *query;

	va_start(ap, q);
	va_copy(cp, ap);
	query = g_strdup_vprintf(q, cp);
	va_end(cp);
	g_strstrip(query);
		
	c = sql_con_get();
	TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
	r = Connection_executeQuery(c, (const char *)query);
	
	return r;
}

int sql_user_exists(char *addr) {
	Connection_T c;
	ResultSet_T r;
	char *query;
//	SETTINGS *settings = g_private_get(settings_key);
	
	c = sql_con_get();
	expand_query(settings->sql_user_query, addr, &query);
	TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
	r = Connection_executeQuery(c,(const char *)query);

	if (ResultSet_next(r)) {
		TRACE(TRACE_LOOKUP, "found user [%s]",addr);
		return 1;
	} else {
		TRACE(TRACE_LOOKUP, "user [%s] does not exist", addr);
		return 0;
	}
}

#endif
