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

#include <sys/types.h>
#include <sys/stat.h>
#include <check.h>
#include <stdlib.h>
#include <unistd.h>

#include "../src/smf_core.h"

START_TEST(strstrip) {
    char *s = strdup(" Test string ");

    fail_unless((s = smf_core_strstrip(s)) != NULL);
    fail_unless(strcmp(s, "Test string") == 0);

    free(s);
}
END_TEST

START_TEST(strlwc) {
    char *s = strdup("Test string");

    fail_unless((s = smf_core_strlwc(s)) != NULL);
    fail_unless(strcmp(s, "test string") == 0);

    free(s);
}
END_TEST

START_TEST(strcat_printf) {
    char *s = strdup("value1");

    fail_unless((smf_core_strcat_printf(&s, ";%s", "value2")) != NULL);
    fail_unless((smf_core_strcat_printf(&s, ";%s", "value3")) != NULL);
    fail_unless(strcmp(s, "value1;value2;value3") == 0);

    free(s);
}
END_TEST

START_TEST(strsplit_no_nelems) {
    const char *src = "value1;value2;value3";
    char **sl = NULL;

    fail_unless((sl = smf_core_strsplit(src, ";", NULL)) != NULL);
    fail_unless(strcmp(sl[0], "value1") == 0);
    fail_unless(strcmp(sl[1], "value2") == 0);
    fail_unless(strcmp(sl[2], "value3") == 0);
    fail_unless(sl[3] == '\0');

    free(sl[0]);
    free(sl[1]);
    free(sl[2]);
    free(sl);
}
END_TEST

START_TEST(strsplit_with_nelems) {
    const char *src = "value1;value2;value3";
    char **sl = NULL;
    int nelems = -99;

    fail_unless((sl = smf_core_strsplit(src, ";", &nelems)) != NULL);
    fail_unless(strcmp(sl[0], "value1") == 0);
    fail_unless(strcmp(sl[1], "value2") == 0);
    fail_unless(strcmp(sl[2], "value3") == 0);
    fail_unless(sl[3] == '\0');
    fail_unless(nelems == 3);

    free(sl[0]);
    free(sl[1]);
    free(sl[2]);
    free(sl);
}
END_TEST

START_TEST(strsplit_no_split) {
    const char *src = "value1";
    char **sl = NULL;
    int nelems = -99;

    fail_unless((sl = smf_core_strsplit(src, ";", &nelems)) != NULL);
    fail_unless(strcmp(sl[0], "value1") == 0);
    fail_unless(sl[1] == '\0');
    fail_unless(nelems == 1);

    free(sl[0]);
    free(sl);
}
END_TEST

START_TEST(gen_queue_file) {
    char *s;
    struct stat fstat;

    fail_unless(smf_core_gen_queue_file("/tmp", &s, "1234567890") == 0);
    fail_unless(stat(s, &fstat) == 0);
    fail_unless(S_ISREG(fstat.st_mode));
    fail_unless(unlink(s) == 0);

    free(s);
}
END_TEST

START_TEST(md5sum) {
    char *s;

    fail_unless((s = smf_core_md5sum("Test string")) != NULL);
    fail_unless(strcmp(s, "0fd3dbec9730101bff92acc820befc34") == 0);

    free(s);
}
END_TEST

START_TEST(get_maildir_filename) {
    char *s;

    fail_unless((s = smf_core_get_maildir_filename()) != NULL);
    free(s);
}
END_TEST

START_TEST(expand_string_unsupported_option) {
    const char *query = "SELECT * FROM users WHERE email='%x'";
    char *s;

	fail_unless(smf_core_expand_string(query, "user@example.org", &s) == -2);
}
END_TEST

START_TEST(expand_string_all) {
    const char *query = "SELECT * FROM users WHERE email='%s'";
    char *s;

    fail_unless(smf_core_expand_string(query, "user@example.org", &s) == 1);
    fail_unless(strcmp(s, "SELECT * FROM users WHERE email='user@example.org'") == 0);

    free(s);
}
END_TEST

START_TEST(expand_string_user) {
    const char *query = "SELECT * FROM users WHERE email='%u'";
    char *s;

    fail_unless(smf_core_expand_string(query, "user@example.org", &s) == 1);
    fail_unless(strcmp(s, "SELECT * FROM users WHERE email='user'") == 0);

    free(s);
}
END_TEST

START_TEST(expand_string_user_no_domain) {
    const char *query = "SELECT * FROM users WHERE email='%u'";
    char *s;

    fail_unless(smf_core_expand_string(query, "user", &s) == 1);
    fail_unless(strcmp(s, "SELECT * FROM users WHERE email='user'") == 0);

    free(s);
}
END_TEST

START_TEST(expand_string_domain) {
    const char *query = "SELECT * FROM users WHERE email='%d'";
    char *s;

    fail_unless(smf_core_expand_string(query, "user@example.org", &s) == 1);
    fail_unless(strcmp(s, "SELECT * FROM users WHERE email='example.org'") == 0);

    free(s);
}
END_TEST

START_TEST(expand_string_domain_no_domain) {
    const char *query = "SELECT * FROM users WHERE email='%d'";
    char *s;

    fail_unless(smf_core_expand_string(query, "user", &s) == -1);

    free(s);
}
END_TEST

START_TEST(expand_string_complex) {
    const char* in = "foo %s bar %d blubb %u%d";
    char *s;
    
    fail_unless(smf_core_expand_string(in, "xxx@example.com", &s) == 4);
    fail_unless(strcmp(s, "foo xxx@example.com bar example.com blubb xxxexample.com") == 0);
}
END_TEST

TCase *core_tcase() {
    TCase* tc = tcase_create("binbuf");

    tcase_add_test(tc, strstrip);
    tcase_add_test(tc, strlwc);
    tcase_add_test(tc, strcat_printf);
    tcase_add_test(tc, strsplit_no_nelems);
    tcase_add_test(tc, strsplit_with_nelems);
    tcase_add_test(tc, strsplit_no_split);
    tcase_add_test(tc, gen_queue_file);
    tcase_add_test(tc, md5sum);
    tcase_add_test(tc, get_maildir_filename);
    tcase_add_test(tc, expand_string_unsupported_option);
    tcase_add_test(tc, expand_string_all);
    tcase_add_test(tc, expand_string_user);
    tcase_add_test(tc, expand_string_user_no_domain);
    tcase_add_test(tc, expand_string_domain);
    tcase_add_test(tc, expand_string_domain_no_domain);
    tcase_add_test(tc, expand_string_complex);

    return tc;
}
