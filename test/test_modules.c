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

#define _GNU_SOURCE

#include <check.h>
#include <stdio.h>

#include "../src/smf_modules.h"
#include "../src/smf_session.h"
#include "../src/smf_session_private.h"

static SMFSession_T *session;
static int mod1_count;

static int mod1(SMFSession_T *s) {
    fail_unless(session == s);
    mod1_count++;
    return 0;
}

static void setup() {
    fail_unless((session = smf_session_new()) != NULL);
    mod1_count = 0;
}

static void teardown() {
    smf_session_free(session);
    session = NULL;
}

START_TEST(create_invoke_destroy_callback) {
    SMFModule_T *module;
    
    fail_unless(mod1_count == 0);
    fail_unless((module = smf_module_create_callback("foo", mod1)) != NULL);
    fail_unless(smf_module_invoke(module, session) == 0);
    fail_unless(smf_module_destroy(module) == 0);
    fail_unless(mod1_count == 1);
}
END_TEST

TCase *modules_tcase() {
    TCase* tc = tcase_create("modules");
    tcase_add_checked_fixture(tc, setup, teardown);
    
    tcase_add_test(tc, create_invoke_destroy_callback);
    
    return tc;
}
