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
#include <string.h>
#include <check.h>

#include "../src/smf_email_address.h"

static SMFEmailAddress_T *ea;

static void setup() {
    fail_unless((ea = smf_email_address_new()) != NULL);
}

static void teardown() {
    smf_email_address_free(ea);
    ea = NULL;
}

START_TEST(set_get_type) {
    smf_email_address_set_type(ea,SMF_EMAIL_ADDRESS_TYPE_TO);
    fail_unless(smf_email_address_get_type(ea) == SMF_EMAIL_ADDRESS_TYPE_TO);
}
END_TEST

START_TEST(set_get_name) {
    char *s = strdup("John Doe");
    smf_email_address_set_name(ea,s);
    fail_unless(strcmp(s,smf_email_address_get_name(ea)) == 0);
    free(s);
}
END_TEST

START_TEST(set_get_email) {
    char *s = strdup("user@example.org");
    smf_email_address_set_email(ea,s);
    fail_unless(strcmp(s,smf_email_address_get_email(ea)) == 0);
    free(s);
}
END_TEST

START_TEST(parse_string) {
    SMFEmailAddress_T *ea2 = NULL;
    char *s = strdup("John Doe <user@example.org>");
    fail_unless((ea2 = smf_email_address_parse_string(s)) != NULL);
    smf_email_address_free(ea2);
    free(s);
}
END_TEST

START_TEST(to_string) {
    SMFEmailAddress_T *ea2 = NULL;
    char *s1 = strdup("John Doe <user@example.org>");
    char *s2;
    fail_unless((ea2 = smf_email_address_parse_string(s1)) != NULL);
    fail_unless(( s2 = smf_email_address_to_string(ea2)) != NULL);
    fail_unless(strcmp(s1, s2) == 0);
    free(s1);
    free(s2);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(is_empty_plain) {
    SMFEmailAddress_T *ea;
    
    fail_unless((ea = smf_email_address_parse_string("   foo@example.com  ")) != NULL);
    fail_unless(smf_email_address_is_empty(ea) == 0);
    smf_email_address_free(ea);

}
END_TEST

START_TEST(is_empty_angle) {
    SMFEmailAddress_T *ea;
    
    fail_unless((ea = smf_email_address_parse_string("Foo      <  foo@example.com    >")) != NULL);
    fail_unless(smf_email_address_is_empty(ea) == 0);
    smf_email_address_free(ea);
}
END_TEST

START_TEST(is_empty_empty_angle) {
    SMFEmailAddress_T *ea;
    
    fail_unless((ea = smf_email_address_parse_string("Foo  <  >")) != NULL);
    fail_unless(smf_email_address_is_empty(ea) == 1);
    smf_email_address_free(ea);
}
END_TEST

START_TEST(is_empty_noname_angle) {
    SMFEmailAddress_T *ea;
    
    fail_unless((ea = smf_email_address_parse_string("    < foo@example.com>")) != NULL);
    fail_unless(smf_email_address_is_empty(ea) == 0);
    smf_email_address_free(ea);
}
END_TEST

START_TEST(is_empty_noname_empty_angle) {
    SMFEmailAddress_T *ea;
    
    fail_unless((ea = smf_email_address_parse_string("   < >")) != NULL);
    fail_unless(smf_email_address_is_empty(ea) == 1);
    smf_email_address_free(ea);
}
END_TEST

START_TEST(simplified_plain) {
    SMFEmailAddress_T *ea1, *ea2;
    
    fail_unless((ea1 = smf_email_address_parse_string("   foo@example.com  ")) != NULL);
    fail_unless((ea2 = smf_email_address_get_simplified(ea1)) != NULL);
    fail_unless(smf_email_address_get_name(ea2) == NULL);
    fail_unless(strcmp(smf_email_address_get_email(ea2), "foo@example.com") == 0);
    smf_email_address_free(ea1);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(simplified_angle) {
    SMFEmailAddress_T *ea1, *ea2;
    
    fail_unless((ea1 = smf_email_address_parse_string("Foo      <  foo@example.com    >")) != NULL);
    fail_unless((ea2 = smf_email_address_get_simplified(ea1)) != NULL);
    fail_unless(smf_email_address_get_name(ea2) == NULL);
    fail_unless(strcmp(smf_email_address_get_email(ea2), "foo@example.com") == 0);
    smf_email_address_free(ea1);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(simplified_empty_angle) {
    SMFEmailAddress_T *ea1, *ea2;
    
    fail_unless((ea1 = smf_email_address_parse_string("Foo  <  >")) != NULL);
    fail_unless((ea2 = smf_email_address_get_simplified(ea1)) != NULL);
    fail_unless(smf_email_address_get_name(ea2) == NULL);
    fail_unless(strcmp(smf_email_address_get_email(ea2), "") == 0);
    smf_email_address_free(ea1);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(simplified_noname_angle) {
    SMFEmailAddress_T *ea1, *ea2;
    
    fail_unless((ea1 = smf_email_address_parse_string("    < foo@example.com>")) != NULL);
    fail_unless((ea2 = smf_email_address_get_simplified(ea1)) != NULL);
    fail_unless(smf_email_address_get_name(ea2) == NULL);
    fail_unless(strcmp(smf_email_address_get_email(ea2), "foo@example.com") == 0);
    smf_email_address_free(ea1);
    smf_email_address_free(ea2);
}
END_TEST

START_TEST(simplified_noname_empty_angle) {
    SMFEmailAddress_T *ea1, *ea2;
    
    fail_unless((ea1 = smf_email_address_parse_string("   < >")) != NULL);
    fail_unless((ea2 = smf_email_address_get_simplified(ea1)) != NULL);
    fail_unless(smf_email_address_get_name(ea2) == NULL);
    fail_unless(strcmp(smf_email_address_get_email(ea2), "") == 0);
    smf_email_address_free(ea1);
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
    tcase_add_test(tc, is_empty_plain);
    tcase_add_test(tc, is_empty_angle);
    tcase_add_test(tc, is_empty_empty_angle);
    tcase_add_test(tc, is_empty_noname_angle);
    tcase_add_test(tc, is_empty_noname_empty_angle);
    tcase_add_test(tc, simplified_plain);
    tcase_add_test(tc, simplified_angle);
    tcase_add_test(tc, simplified_empty_angle);
    tcase_add_test(tc, simplified_noname_angle);
    tcase_add_test(tc, simplified_noname_empty_angle);

    return tc;
}
