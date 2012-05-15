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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <db.h>
#include <glib/gprintf.h>
#include "../src/smf_lookup.h"

#define TESTDB "/tmp/smf_test_db4_11.db"


int remove_db(void) {
    if((remove(TESTDB)) < 0) {
      fprintf(stderr, "Fehler beim Löschen von %s", TESTDB);
      return(-1);
    }
    return(0);
}


int main (int argc, char const *argv[]) {
    
    DB *dbp;
    DBT key, data;
    u_int32_t flags;  
    flags = DB_CREATE;
    int ret;

    char *value_str = "this is a test";
    char *key_char = "2323";
    char *res_from_db;

    /* first create a new berkeley database */
    if ((ret = db_create(&dbp, NULL, 0)) != 0) {
        g_printf("db_create: %s\n", db_strerror(ret));
        return(-1);
    }
    /* set page- and cachesize */
    if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
        g_printf("db->open: %s\n", db_strerror(ret));
        return(-1);
    }
    if ((ret = dbp->set_cachesize(dbp, 0, 32 * 1024, 0)) != 0) {
        g_printf("db->open: %s\n", db_strerror(ret));
        return(-1);
    }
    /* open the database */
    if((ret = dbp->open(dbp, NULL, TESTDB, NULL, DB_HASH, flags, 0)) != 0) {
        g_printf("db->open: %s\n", db_strerror(ret));
        return(-1);
    }
    /* write to database, zero out the DBTs before using them */
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = key_char;
    key.size = strlen(key_char)+1;
    data.data = value_str;
    data.size = strlen(value_str)+1; 

    if((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
        g_printf("db->put: %s\n", db_strerror(ret));
        return(-1);
    }
    /* close the database */
    if (dbp != NULL)
        dbp->close(dbp, 0); 

    /* read out database with spmfilter function and compare the values */ 
    res_from_db = smf_lookup_db4_query(TESTDB, key_char);
    if(res_from_db != NULL) {
        if(strcmp(res_from_db,value_str) != 0) {
            g_printf("received different value from db");
            remove_db();
            return(-1);
        } else {
            remove_db();
            return(0);
        }
    } else {
        g_printf("nothing received by smf_lookup_db4_query()");
        remove_db();
        return(-1);
    }
    return(0);
}

