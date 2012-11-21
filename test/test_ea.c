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
 *
 * Original implementation by N.Devillard
 */

#include <stdlib.h>
#include <check.h>

#include "../src/smf_email_address.h"

static SMFEmailAddress_T *ea;

static void setup() {
    fail_unless((ea = smf_email_address_new()) != NULL);
}

static void teardown() {
    smf_email_address_free(ea);
}

START_TEST(set_get_type) {
    smf_email_address_set_type(ea,SMF_EMAIL_ADDRESS_TYPE_TO);
    fail_unless(smf_email_address_get_type(ea) == SMF_EMAIL_ADDRESS_TYPE_TO);
}
END_TEST

START_TEST(set_get_name) {
    smf_email_address_set_name(ea,"John Doe");
    fail_unless(strcmp("John Doe",smf_email_address_get_name(ea)) == 0);
}
END_TEST

START_TEST(set_get_email) {
    smf_email_address_set_email(ea,"user@example.org");
    fail_unless(strcmp("user@example.org",smf_email_address_get_email(ea)) == 0);
}
END_TEST

START_TEST(parse_string) {
    SMFEmailAddress_T *ea2 = NULL;
    fail_unless((ea2 = smf_email_address_parse_string("John Doe <user@example.org>")) != NULL);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(to_string) {
    SMFEmailAddress_T *ea2 = NULL;
    char *s;
    fail_unless((ea2 = smf_email_address_parse_string("John Doe <user@example.org>")) != NULL);
    fail_unless(( s = smf_email_address_to_string(ea2)) != NULL);
    fail_unless(strcmp(s, "John Doe <user@example.org>") == 0);
    free(s);
    smf_email_address_free(ea2);
}
END_TEST

TCase *ea_tcase() {
    TCase* tc = tcase_create("email_address");

    tcase_add_checked_fixture(tc, setup, teardown);

    tcase_add_test(tc, set_get_type);
    tcase_add_test(tc, set_get_name);
    tcase_add_test(tc, set_get_email);
    tcase_add_test(tc, parse_string);
    tcase_add_test(tc, to_string);

    return tc;
}
