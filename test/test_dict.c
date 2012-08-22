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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../src/smf_dict.h"
#include "../src/smf_list.h"

#include "test.h"

int iter_ok = 0;

void _iter_test(char *key, char *value, void *args) {
    if ((strcmp(key,test_header_name)==0)||(strcmp(key,test_header_value)==0)) {
        if (strcmp(value,test_header_value)==0) {
            iter_ok++;
        }
    }
}

int main(int argc, char const *argv[]) {
    SMFDict_T *dict = NULL;
    SMFList_T *l = NULL;
    SMFListElem_T *e = NULL;
    char *s = NULL;
    int count = 0;

    printf("Start SMFDict_T tests...\n");

    printf("* testing smf_dict_new()...\t\t\t");
    dict = smf_dict_new();
    assert(dict);
    if (smf_dict_count(dict) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_dict_set()...\t\t\t");
    if (smf_dict_set(dict,test_header_name,test_header_value)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_dict_get()...\t\t\t");
    if (strcmp(smf_dict_get(dict,test_header_name),test_header_value)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_dict_get_keys()...\t\t");
    if (smf_dict_set(dict,test_header_value,test_header_value)!=0) {
        printf("failed\n");
        return -1;
    }

    l = smf_dict_get_keys(dict);
    e = smf_list_head(l);
    while(e != NULL) {
        s = smf_list_data(e);
        if ((strcmp(s,test_header_name)!=0)&&(strcmp(s,test_header_value)!=0)) {
            printf("failed\n");
            return -1;
        }
        count++;
        e = e->next;
    }
    if (smf_dict_count(dict) != 2) {
        printf("failed\n");
        return -1;
    }
    if (smf_list_free(l)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_dict_map()...\t\t\t");
    smf_dict_map(dict,_iter_test,NULL);
    if (iter_ok==2)
        printf("passed\n");
    else {
        printf("failed\n");
        return -1;
    }

    printf("* testing smf_dict_remove()...\t\t\t");
    smf_dict_remove(dict,test_header_value);

    if (smf_dict_count(dict) != 1) {
        printf("failed\n");
        return -1;
    }    

    if (smf_dict_get(dict,test_header_value)==NULL) {
        printf("passed\n");
    } else {
        printf("failed\n");
        return -1;
    }

    printf("* testing smf_dict_free()...\t\t\t");
    smf_dict_free(dict);
    printf("passed\n");

    return 0;
}