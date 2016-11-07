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
#define _GNU_SOURCE

#include <string.h>
#include <db.h>

#include "smf_trace.h"
#include "smf_settings.h"

#define THIS_MODULE "lookup_db4"


char *smf_lookup_db4_query(char *database, char *key) {
    DB *dbp;
    DBT db_key, db_value;
    int ret;
    char *db_res = NULL;

    /* initialize db4 */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        TRACE(TRACE_ERR, "db_create: %s\n", db_strerror(ret));
        return NULL;
    }

    TRACE(TRACE_LOOKUP, "[%p] lookup key [%s]", dbp,key);

    if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
        TRACE(TRACE_WARNING, "DB (%s): %s", database, db_strerror(ret));
    }
    if ((ret = dbp->set_cachesize(dbp, 0, 32 * 1024, 0)) != 0) {
        TRACE(TRACE_WARNING, "DB (%s): %s", database, db_strerror(ret));
    }

    /* open db */
#if DB_VERSION_MAJOR >= 4 && DB_VERSION_MINOR < 1
    if ((ret = dbp->open(dbp, database, NULL, DB_HASH, DB_RDONLY, 0)) != 0) {
        TRACE(TRACE_ERR, "DB (%s): %s", database, db_strerror(ret));
        return NULL;
    }
#else
    if ((ret = dbp->open(dbp, NULL, database, NULL, DB_HASH, DB_RDONLY, 0)) != 0) {
        TRACE(TRACE_ERR,"DB (%s): %s", database, db_strerror(ret));
        return NULL;
    }
#endif
    else {
        TRACE(TRACE_DEBUG, "DB (%s): open", database);
    }

    memset(&db_key, 0, sizeof(DBT));
    memset(&db_value, 0, sizeof(DBT));
    db_key.data = (void *)key;
    db_key.size = strlen(key) + 1;

    ret = dbp->get(dbp, NULL, &db_key, &db_value, 0);
    
    if (ret == 0) {
        if (asprintf(&db_res, "%s", (char *)db_value.data) != -1)
            TRACE(TRACE_LOOKUP, "[%p] found value [%s]", dbp, db_res);
    } else
        TRACE(TRACE_LOOKUP, "[%p] nothing found", dbp);

    if (dbp != NULL)
        dbp->close(dbp, 0);

    return db_res;
}

int smf_lookup_db4_update(const char *database, const char *key, const char *value) {
    DB *dbp;
    DBT db_key, db_data;
    int ret;

    /* Initialize the structure. This database is not opened in an environment,
     * so the environment pointer is NULL. */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        TRACE(TRACE_ERR, "db_create: %s\n", db_strerror(ret));
        return -1;
    }

    /* set page- and cachesize */
    if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
        printf("db->open: %s\n", db_strerror(ret));
        return(-1);
    }
    if ((ret = dbp->set_cachesize(dbp, 0, 32 * 1024, 0)) != 0) {
        printf("db->open: %s\n", db_strerror(ret));
        return(-1);
    }

    /* open the database */
    ret = dbp->open(dbp, NULL, database, NULL, DB_HASH, DB_CREATE, 0);
    if (ret != 0) {
        TRACE(TRACE_ERR,"DB: %s",db_strerror(ret));
        return -1;
    }

    memset(&db_key, 0, sizeof(DBT));
    db_key.data = (char*)key;
    db_key.size = strlen(key) + 1;
    memset(&db_data, 0, sizeof(DBT));
    db_data.data = (char*)value;
    db_data.size = strlen(value) + 1;

    ret = dbp->put(dbp, NULL, &db_key, &db_data, 0);
    if (ret == 0) {
        TRACE(TRACE_DEBUG, "[%p] update was successful [%s: %s]", dbp, key, value);
    } else {
        TRACE(TRACE_ERR, "[%p] failed to update database [%s: %s]: %s",
              dbp, key, value, db_strerror(ret));
    }

    if (dbp != NULL)
        dbp->close(dbp, 0);

    return (ret == 0) ? ret : -1;
}

