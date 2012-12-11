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

#include <check.h>

#include "../src/smf_dict.h"
#include "../src/smf_list.h"

static SMFDict_T *dict;

int iter_ok = 0;

static void iter_test(char *key, char *value, void *args) {
    if ((strcmp(key,"key1")==0)||(strcmp(key,"key2")==0)) {
        if (strcmp(value,"value")==0) {
            iter_ok++;
        }
    }
}

static void setup() {
    fail_unless((dict = smf_dict_new()) != NULL);
}

static void teardown() {
    smf_dict_free(dict);
    dict = NULL;
}

START_TEST(set) {
    fail_unless(smf_dict_set(dict,"key","value")==0);
}
END_TEST

START_TEST(get) {
    fail_unless(smf_dict_set(dict,"key","value")==0);
    fail_unless(strcmp(smf_dict_get(dict,"key"),"value")==0);
}
END_TEST

START_TEST(get_fail) {
    fail_unless(smf_dict_get(dict,"notexisting")==NULL);
}
END_TEST

START_TEST(keys) {
    SMFList_T *l = NULL;
    SMFListElem_T *e = NULL;
    char *s;

    fail_unless(smf_dict_set(dict,"key1","value1")==0);
    fail_unless(smf_dict_set(dict,"key2","value2")==0);
    fail_unless((l = smf_dict_get_keys(dict))!=NULL);
    
    e = smf_list_head(l);
    while(e != NULL) {
        s = smf_list_data(e);
        fail_unless((strcmp(s,"key1")==0)||(strcmp(s,"key2")==0));
        e = e->next;
    }
    fail_unless(smf_dict_count(dict)==2);
    smf_list_free(l);
}
END_TEST

START_TEST(map) {
    fail_unless(smf_dict_set(dict,"key1","value")==0);
    fail_unless(smf_dict_set(dict,"key2","value")==0);
    smf_dict_map(dict,iter_test,NULL);
    fail_unless(iter_ok==2);

}
END_TEST

START_TEST(remove_item) {
    fail_unless(smf_dict_set(dict,"key","value")==0);
    smf_dict_remove(dict,"key");

    fail_unless(smf_dict_count(dict)==0);
}
END_TEST

START_TEST(get_ulong_empty) {
    int success;
    fail_unless(smf_dict_get_ulong(dict, "foo", &success)  == -1);
    fail_unless(success == 0);
    fail_unless(smf_dict_get_ulong(dict, "foo", NULL)  == -1);
}
END_TEST

START_TEST(get_ulong_no_num) {
    int success;
    fail_unless(smf_dict_set(dict, "foo", "1234bar") == 0);
    fail_unless(smf_dict_get_ulong(dict, "foo", &success)  == -1);
    fail_unless(success == 0);
    fail_unless(smf_dict_get_ulong(dict, "foo", NULL)  == -1);
}
END_TEST

START_TEST(get_ulong) {
    int success;
    fail_unless(smf_dict_set(dict, "foo", "4711") == 0);
    fail_unless(smf_dict_get_ulong(dict, "foo", &success)  == 4711);
    fail_unless(success == 1);
    fail_unless(smf_dict_get_ulong(dict, "foo", NULL)  == 4711);
}
END_TEST

TCase *dict_tcase() {
    TCase* tc = tcase_create("dict");

    tcase_add_checked_fixture(tc, setup, teardown);

    tcase_add_test(tc, set);
    tcase_add_test(tc, get);
    tcase_add_test(tc, get_fail);
    tcase_add_test(tc, keys);
    tcase_add_test(tc, map);
    tcase_add_test(tc, remove_item);
    tcase_add_test(tc, get_ulong_empty);
    tcase_add_test(tc, get_ulong_no_num);
    tcase_add_test(tc, get_ulong);

    return tc;
}
