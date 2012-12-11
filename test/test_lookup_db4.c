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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../src/smf_lookup.h"

#define TESTDB "/tmp/smf_test_db4.db"

int remove_db(void) {
    if((remove(TESTDB)) < 0 && errno != ENOENT) {
      fprintf(stderr, "Fehler beim Löschen von %s", TESTDB);
      return(-1);
    }
    return(0);
}

int main (int argc, char const *argv[]) {
    char *value_str_1 = "this is a test";
    char *value_str_2 = "this is a second test";
    char *key_char = "2323";
    char *res_from_db = NULL;

    printf("Start smf_lookup_db4 tests...\n");
    remove_db();

    /* first create a new berkeley database */
    printf("* testing smf_lookup_db4_update() (1)...\t\t\t");
    assert(smf_lookup_db4_update(TESTDB, key_char, value_str_1) == 0);
    printf("passed\n");

    /* read out database with spmfilter function and compare the values */ 
    printf("* testing smf_lookup_db4_query() (1)...\t\t\t");
    assert((res_from_db = smf_lookup_db4_query(TESTDB, key_char)) != NULL);
    assert(strcmp(res_from_db,value_str_1) == 0);
    free(res_from_db);
    printf("passed\n");
    
    /* Update an already existing key */
    printf("* testing smf_lookup_db4_update() (2)...\t\t\t");
    assert(smf_lookup_db4_update(TESTDB, key_char, value_str_2) == 0);
    printf("passed\n");

    /* read out database again */ 
    printf("* testing smf_lookup_db4_query() (2)...\t\t\t");
    assert((res_from_db = smf_lookup_db4_query(TESTDB, key_char)) != NULL);
    assert(strcmp(res_from_db,value_str_2) == 0);
    free(res_from_db);
    printf("passed\n");

    remove_db();

    return(0);
}

