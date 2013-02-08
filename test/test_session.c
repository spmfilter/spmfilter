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

#include "../src/smf_session.h"

static SMFSession_T *session;

static void setup() {
    fail_unless((session = smf_session_new()) != NULL);
}

static void teardown() {
    smf_session_free(session);
    session = NULL;
}

START_TEST(set_get_helo) {
    char *s = strdup("foo.bar");
    smf_session_set_helo(session, s);
    fail_unless(strcmp(s,smf_session_get_helo(session)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_response_msg) {
    char *s = strdup("250 OK message accepted");
    smf_session_set_response_msg(session, s);
    fail_unless(strcmp(s,smf_session_get_response_msg(session)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_message_file) {
    char *s = strdup("/tmp/test.eml");
    smf_session_set_message_file(session,s);
    fail_unless(strcmp(s,smf_session_get_message_file(session)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_xforward_v4) {
    char *s = strdup("127.0.0.1");
    smf_session_set_xforward_addr(session, s);
    fail_unless(strcmp(s,smf_session_get_xforward_addr(session)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_xforward_v6) {
    char *s = strdup("IPv6:::1");
    smf_session_set_xforward_addr(session, s);
    fail_unless(strcmp("::1",smf_session_get_xforward_addr(session)) == 0);
    free(s);
}
END_TEST

TCase *session_tcase() {
    TCase* tc = tcase_create("session");

    tcase_add_checked_fixture(tc, setup, teardown);

    tcase_add_test(tc, set_get_helo);
    tcase_add_test(tc, set_get_response_msg);
    tcase_add_test(tc, set_get_message_file);
    tcase_add_test(tc, set_get_xforward_v4);
    tcase_add_test(tc, set_get_xforward_v6);

    return tc;
}


#if 0
#include "test.h"

int main (int argc, char const *argv[]) {
    SMFSession_T *session = NULL;
    
    printf("Start SMFSession_T tests...\n");

    printf("* testing smf_session_new()...\t\t\t\t");
    session = smf_session_new();
    assert(session);
    if (strlen(smf_session_get_id(session)) != 12) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_session_set_helo()...\t\t\t");
    smf_session_set_helo(session,test_helo);
    printf("passed\n");

    printf("* testing smf_session_get_helo()...\t\t\t");
    if (strcmp(test_helo,smf_session_get_helo(session)) != 0) {
        printf("failed\n");
        return -1;
    } else
        printf("passed\n");
      
    printf("* testing smf_session_set_xfoward_addr()...\t\t");
    smf_session_set_xforward_addr(session,test_ip);
    printf("passed\n");

    printf("* testing smf_session_get_xforward_addr()...\t\t");
    if (strcmp(test_ip,smf_session_get_xforward_addr(session)) != 0) {
        printf("failed\n");
        return -1;
    } else
        printf("passed\n");

    printf("* testing smf_session_set_response_msg()...\t\t");
    smf_session_set_response_msg(session,test_response);
    printf("passed\n");

    printf("* testing smf_session_get_response_msg()...\t\t");
    if (strcmp(test_response,smf_session_get_response_msg(session)) != 0) {
        printf("failed\n");
        return -1;
    } else
        printf("passed\n");
  
    printf("* testing smf_session_set_message_file()...\t\t");
    smf_session_set_message_file(session,test_path);
    printf("passed\n");

    printf("* testing smf_session_get_message_file()...\t\t");
    if (strcmp(test_path,smf_session_get_message_file(session)) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_session_free()...\t\t\t\t");
    smf_session_free(session);
    printf("passed\n");

    return 0;
}
#endif
