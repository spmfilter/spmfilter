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
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_envelope.h"

#define TEST_SENDER "webmaster@spmfilter.org"
#define TEST_RCPT1 "ast@spmfilter.org"
#define TEST_RCPT2 "postmaster@spmfilter.org"
#define TEST_PATH "/tmp/test.eml"
#define TEST_AUTH_USER "testusername"
#define TEST_AUTH_PASS "testpassword"
#define TEST_NEXTHOP "localhost:2525"

int foreach_rc = -1;

#if 0
static void print_rcpt_func(SMFEmailAddress_T *ea, void *data) {
    if ((strcmp(ea->addr,TEST_RCPT1) != 0) && 
            (strcmp(ea->addr,TEST_RCPT2) != 0)) {
        foreach_rc = -1;
    } else {
        foreach_rc = 0;
    }
}
#endif

int main (int argc, char const *argv[]) {
    SMFEnvelope_T *env = NULL;
    g_printf("Start SMFEnvelope_T tests...\n");
    g_printf("* testing smf_envelope_new()...\t\t\t");
    env = smf_envelope_new();
    assert(env);
    g_printf("passed\n");
#if 0    
    g_printf("* testing smf_envelope_set_sender()...\t\t");
    smf_envelope_set_sender(env,TEST_SENDER);
    if (strcmp(TEST_SENDER,smf_envelope_get_sender(env)->addr) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
    
    g_printf("* testing smf_envelope_add_rcpt()...\t\t");
    smf_envelope_add_rcpt(env,TEST_RCPT1);
    smf_envelope_add_rcpt(env,TEST_RCPT2);
    if (env->num_rcpts != 2) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
     

    g_printf("* testing smf_envelope_foreach_rcpt()...\t");
    smf_envelope_foreach_rcpt(env, print_rcpt_func, NULL);
    if (foreach_rc != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    
    g_printf("* testing smf_envelope_set_message_file()...\t");
    smf_envelope_set_message_file(env,TEST_PATH);
    if (strcmp(TEST_PATH,smf_envelope_get_message_file(env)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
    

    g_printf("* testing smf_envelope_set_auth_user()...\t");
    smf_envelope_set_auth_user(env,TEST_AUTH_USER);
    if (strcmp(TEST_AUTH_USER,smf_envelope_get_auth_user(env)) != 0) {
        g_printf("failed\n");
        return -1;
    } else 
        g_printf("passed\n");

    g_printf("* testing smf_envelope_set_auth_pass()...\t");
    smf_envelope_set_auth_pass(env,TEST_AUTH_PASS);
    if (strcmp(TEST_AUTH_PASS,smf_envelope_get_auth_pass(env)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    g_printf("* testing smf_envelope_set_nexthop()...\t\t");
    smf_envelope_set_nexthop(env,TEST_NEXTHOP);
    if (strcmp(TEST_NEXTHOP,smf_envelope_get_nexthop(env)) != 0) {
        g_printf("failed\n");
        return -1;
    } else 
        g_printf("passed\n");
#endif    
    g_printf("* testing smf_envelope_free()...\t\t");
    smf_envelope_free(env);
    g_printf("passed\n");
    return 0;
}
