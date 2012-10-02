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

#include <stdio.h>
#include <string.h>

#include "../src/smf_core.h"
#include "../src/smf_list.h"
#include "test.h"

int main (int argc, char const *argv[]) {
    char *s = NULL;
    char **sl = NULL;
    
    printf("Start smf_core tests...\n");

    printf("* testing smf_core_strstrip()...\t\t\t");
    s = strdup(test_string_strip);
    smf_core_strstrip(s);
    if (strcmp(s,test_string)!=0) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");

    printf("* testing smf_core_strlwc()...\t\t\t\t");
    s = strdup(test_string);
    smf_core_strlwc(s);
    if (strcmp(s,test_string_lower)!=0) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");

    printf("* testing smf_core_strcat_printf()...\t\t\t");
    s = strdup(test_split_value1);
    smf_core_strcat_printf(&s, ";%s", test_split_value2);
    smf_core_strcat_printf(&s, ";%s", test_split_value3);
    if (strcmp(s,test_split)!=0) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");

    printf("* testing smf_core_strsplit()...\t\t\t");
    sl = smf_core_strsplit(test_split,";");
    if ((strcmp(sl[0],test_split_value1)!=0) 
            || (strcmp(sl[1],test_split_value2)!=0) 
            || (strcmp(sl[2],test_split_value3)!=0)) {
        printf("failed\n");
        return -1;
    }
    free(sl[0]);
    free(sl[1]);
    free(sl[2]);
    free(sl);
    printf("passed\n");
    
    printf("* testing smf_core_gen_queue_file()...\t\t\t");
    if (smf_core_gen_queue_file(P_tmpdir,&s,"1234567890") != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    free(s);

    printf("* testing smf_core_md5sum()...\t\t\t\t");
    s = smf_core_md5sum(test_string);
    if (strcmp(s,test_string_md5)!=0) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");

    printf("* testing smf_core_get_maildir_filanem()...\t\t");
    s = smf_core_get_maildir_filename();
    if (s == NULL) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");

    printf("* testing smf_core_expand_string()...\t\t\t");
    if (smf_core_expand_string(test_sql_expand_query,test_email,&s)==-1) {
        printf("failed\n");
        return -1;
    }
    if (strcmp(s,test_sql_expand_query_out)!=0) {
        printf("failed\n");
        return -1;
    }
    free(s);
    printf("passed\n");


    return 0;
}
