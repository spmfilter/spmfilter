#ifdef HAVE_ZDB
#include <glib.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#include <ConnectionPool.h>
#include <SQLException.h>

#include "spmfilter.h"

void sql_con_close(SETTINGS *settings, Connection_T c) {
	if (settings->debug)
		syslog(LOG_DEBUG,"[%p] connection to pool", c);
	Connection_close(c);
	return;
}

int sql_connect(SETTINGS *settings) {
	URL_T url = NULL;
	Connection_T con = NULL;
	GString *dsn = g_string_new("");
	int sweep_interval = 60;

	if (settings->sql_driver != NULL) {
		g_string_append_printf(dsn,"%s://",settings->sql_driver);
	} else {
		syslog(LOG_ERR,"Warning, no sql driver defined!");
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
					syslog(LOG_ERR,"can't expand ~ in db name");
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

	if (settings->debug)
		syslog(LOG_DEBUG,"sql db at url: [%s]", dsn->str);
	
	
	url = URL_new(dsn->str);
	g_string_free(dsn,TRUE);
	
	if (! (settings->sql_pool = ConnectionPool_new(url)))
		syslog(LOG_ERR,"error creating database connection pool");
	
	if (settings->sql_max_connections > 0) {
		if (settings->sql_max_connections < (unsigned int)ConnectionPool_getInitialConnections(settings->sql_pool))
			ConnectionPool_setInitialConnections(settings->sql_pool, settings->sql_max_connections);
		ConnectionPool_setMaxConnections(settings->sql_pool, settings->sql_max_connections);
		if (settings->debug)
			syslog(LOG_DEBUG,"database connection pool created with maximum connections of [%d]",settings->sql_max_connections);
	}

	ConnectionPool_setReaper(settings->sql_pool, sweep_interval);
	if (settings->debug)
		syslog(LOG_DEBUG, "run a database connection reaper thread every [%d] seconds", sweep_interval);

	ConnectionPool_start(settings->sql_pool);
	if (settings->debug)
		syslog(LOG_DEBUG, "database connection pool started with [%d] connections, max [%d]", 
			ConnectionPool_getInitialConnections(settings->sql_pool), ConnectionPool_getMaxConnections(settings->sql_pool));

	if (! (con = ConnectionPool_getConnection(settings->sql_pool))) {
		sql_con_close(settings,con);
		syslog(LOG_ERR, "error getting a database connection from the pool");
		return -1;
	}
	sql_con_close(settings,con);

	return 0;
}

Connection_T db_con_get(SETTINGS *settings) {
	int i=0, k=0; 
	Connection_T c;
	while (i++<30) {
		c = ConnectionPool_getConnection(settings->sql_pool);
		if (c) break;
		if((int)(i % 5)==0) {
			syslog(LOG_WARNING, "Thread is having trouble obtaining a database connection. Try [%d]", i);
			k = ConnectionPool_reapConnections(settings->sql_pool);
			if (settings->debug)
				syslog(LOG_DEBUG, "Database reaper closed [%d] stale connections", k);
		}
		sleep(1);
	}
	if (! c) {
		syslog(LOG_ERR,"[%p] can't get a database connection from the pool! max [%d] size [%d] active [%d]", 
			settings->sql_pool,
			ConnectionPool_getMaxConnections(settings->sql_pool),
			ConnectionPool_size(settings->sql_pool),
			ConnectionPool_active(settings->sql_pool));
	}

	assert(c);
	if (settings->debug)
		syslog(LOG_DEBUG,"[%p] connection from pool", c);
	return c;
}


#endif
