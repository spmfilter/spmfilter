/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
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

#include <stdio.h>
#include <assert.h>

#include "../src/smf_list.h"

#define TEST_STRING1 "Test string 1"
#define TEST_STRING2 "Test string 2"
#define TEST_STRING3 "Test string 3"
#define TEST_STRING4 "Test string 4"

void list_char_printer(SMFListElem_T *elem,void *args) {
    assert(elem);
}

void list_destroy(void *data) {
    assert(data);
}

int main (int argc, char const *argv[]) {
    SMFList_T *l;
    char *out;
    char *data;
    char *pop;
    SMFListElem_T *e;

    printf("Start SMFList_T tests...\n");

    printf("* testing smf_list_new()...\t\t\t\t");
    if (smf_list_new(&l,list_destroy)!=0) {
        printf("failed\n");           
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_append()...\t\t\t\t");
    if (smf_list_append(l,TEST_STRING1)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_list_data()...\t\t\t\t");
    out = (char *)smf_list_data(smf_list_head(l));
    if (strcmp(TEST_STRING1,out)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_list_size()...\t\t\t\t");
    if (smf_list_size(l)!=1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_list_head()...\t\t\t\t");
    e = smf_list_head(l);
    if (smf_list_is_head(e)!=1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_insert_next()...\t\t\t");
    if (smf_list_insert_next(l,e,TEST_STRING2)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_list_tail()...\t\t\t\t");
    e = smf_list_tail(l);
    if (smf_list_is_tail(e)!=1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_data()...\t\t\t\t");
    out = (char *)smf_list_data(e);
    if (strcmp(TEST_STRING2,out)!=0) { 
        printf("failed\n");
        return -1;
    } 
    printf("passed\n");
    
    printf("* testing smf_list_insert_prev()...\t\t\t");    
    if (smf_list_insert_prev(l,e,TEST_STRING3)!=0) {
        printf("failed\n");
        return -1;
    }

    out = (char *)smf_list_data(smf_list_prev(e));
    if (strcmp(TEST_STRING3,out)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");


    printf("* testing smf_list_prepend()...\t\t\t\t");
    if (smf_list_prepend(l,TEST_STRING4)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_map()...\t\t\t\t");
    smf_list_map(l,list_char_printer,NULL);
    printf("passed\n");

    printf("* testing smf_list_remove()...\t\t\t\t");
    e = smf_list_head(l);
    if (smf_list_remove(l,e,(void *)&data)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_pop_head()...\t\t\t");
    pop = smf_list_pop_head(l);
    assert(pop);
    if (strcmp(pop,TEST_STRING1)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_list_pop_tail()...\t\t\t");
    pop = smf_list_pop_tail(l);
    assert(pop);
    if (strcmp(pop,TEST_STRING2)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_list_free()...\t\t\t\t");
    if (smf_list_free(l)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    return 0;
}