/* spmfilter - mail filtering framework
 * Copyright (C) 2012 Axel Steiner and SpaceNet AG
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
#include <check.h>
#include <string.h>

#include "../src/smf_envelope.h"
#include "../src/smf_email_address.h"
#include "../src/smf_message.h"

static SMFEnvelope_T *env;

int foreach_rc = -1;

static void print_rcpt_func(char *s, void *data) {
    if (strcmp(s,"user@example.org") != 0) {
        foreach_rc = -1;
    } else {
        foreach_rc = 0;
    }
}

static void setup() {
    fail_unless((env = smf_envelope_new()) != NULL);
}

static void teardown() {
    smf_envelope_free(env);
    env = NULL;
}

START_TEST(set_get_sender) {
    smf_envelope_set_sender(env,"<user@example.org>");
    fail_unless(strcmp(smf_envelope_get_sender(env),"user@example.org") == 0);
}
END_TEST

START_TEST(set_get_auth_user) {
    char *s = strdup("test@localhost");
    smf_envelope_set_auth_user(env,s);
    fail_unless(strcmp(s,smf_envelope_get_auth_user(env)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_auth_pass) {
    char *s = strdup("test123");
    smf_envelope_set_auth_pass(env,s);
    fail_unless(strcmp(s,smf_envelope_get_auth_pass(env)) == 0);
    free(s);
}
END_TEST


START_TEST(set_get_nexthop) {
    char *s = strdup("localhost:2525");
    smf_envelope_set_nexthop(env,s);
    fail_unless(strcmp(s,smf_envelope_get_nexthop(env)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_message) {
    SMFMessage_T *msg = NULL;
    char *s1 = strdup("John Doe <user@example.org>");
    char *s2 = strdup("Jane Doe <user@example.org>");
    char *s3 = strdup("Test string");

    msg = smf_message_create_skeleton(s1, s2, s3);
    smf_envelope_set_message(env,msg);
    
    fail_unless((msg = smf_envelope_get_message(env)) != NULL);

    free(s1);
    free(s2);
    free(s3);
}
END_TEST

START_TEST(add_rcpt) {
    smf_envelope_add_rcpt(env,"user@example.org");

    fail_unless(env->recipients->size == 1); 
    smf_envelope_foreach_rcpt(env, print_rcpt_func, NULL);
    fail_unless(foreach_rc == 0);
}
END_TEST

TCase *env_tcase() {
    TCase* tc = tcase_create("envelope");

    tcase_add_checked_fixture(tc, setup, teardown);

    tcase_add_test(tc, set_get_sender);
    tcase_add_test(tc, set_get_auth_user);
    tcase_add_test(tc, set_get_auth_pass);
    tcase_add_test(tc, set_get_nexthop);
    tcase_add_test(tc, set_get_message);
    tcase_add_test(tc, add_rcpt);

    return tc;
}
