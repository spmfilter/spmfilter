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

#define _GNU_SOURCE
#include <stdio.h>
#include <check.h>

#include "../src/smf_message.h"
#include "../src/smf_list.h"
#include "../src/smf_header.h"
#include "../src/smf_part.h"

#include "testdirs.h"

static const char *test_files[] = {
    "m0001.txt","m0002.txt","m0003.txt","m0004.txt","m0005.txt",
    "m0006.txt","m0007.txt","m0008.txt","m0009.txt","m0010.txt",
    "m0011.txt","m0012.txt","m0013.txt","m0014.txt","m0015.txt",
    "m0016.txt","m0017.txt","m0018.txt","m1001.txt","m1002.txt",
    "m1003.txt","m1004.txt","m1005.txt","m1006.txt","m1007.txt",
    "m1008.txt","m1009.txt","m1010.txt","m1011.txt","m1012.txt",
    "m1013.txt","m1014.txt","m1015.txt","m1016.txt","m2001.txt",
    "m2002.txt","m2003.txt","m2004.txt","m2005.txt","m2006.txt",
    "m2007.txt","m2008.txt","m2009.txt","m2010.txt","m2011.txt",
    "m2012.txt","m2013.txt","m2014.txt","m2015.txt","m2016.txt",
    "m3001.txt","m3002.txt","m3003.txt","m3004.txt"
};

static SMFMessage_T *msg;

static void setup() {
    fail_unless((msg = smf_message_new()) != NULL);
}

static void teardown() {
    smf_message_free(msg);
    msg = NULL;
}

START_TEST(set_sender) {
    SMFEmailAddress_T *sender;

    smf_message_set_sender(msg, "Foo <foo@example.com>");
    sender = smf_message_get_sender(msg);
    ck_assert_str_eq(smf_email_address_get_email(sender), "<foo@example.com>");
    ck_assert_str_eq(smf_email_address_get_name(sender), "Foo ");
}
END_TEST

START_TEST(get_sender_string) {
    char *sender;

    smf_message_set_sender(msg, "foo@example.com");
    sender = smf_message_get_sender_string(msg);
    ck_assert_str_eq(sender, "foo@example.com");
    free(sender);
}
END_TEST

START_TEST(generate_message_id) {
    char *mid;
    
    mid = smf_message_generate_message_id();
    fail_unless(mid != NULL);
    free(mid);
}
END_TEST

START_TEST(set_message_id) {
    smf_message_set_message_id(msg, "foobar");
    ck_assert_str_eq(smf_message_get_message_id(msg), "<foobar>");
}
END_TEST

START_TEST(set_header) {
    SMFHeader_T *hdr;

    fail_unless(smf_message_set_header(msg, "X-Foo: foobar") == 0);
    fail_unless((hdr = smf_message_get_header(msg, "X-Foo")) != NULL);
    ck_assert_str_eq(smf_header_get_name(hdr), "X-Foo");
    ck_assert_str_eq(smf_header_get_value(hdr, 0), "foobar");
}
END_TEST

START_TEST(remove_header) {
    SMFHeader_T *hdr;

    fail_unless(smf_message_set_header(msg, "X-Foo: foobar") == 0);
    fail_unless((hdr = smf_message_get_header(msg, "X-Foo")) != NULL);
    fail_unless(smf_message_remove_header(msg, "X-Foo") == 0);
    fail_unless(smf_message_get_header(msg, "X-Foo") == NULL);
}
END_TEST

START_TEST(add_recipient) {
    SMFList_T *l;
    SMFListElem_T *e;

    fail_unless(smf_message_add_recipient(msg, "foo@example.com", SMF_EMAIL_ADDRESS_TYPE_TO) == 0);
    fail_unless(smf_message_add_recipient(msg, "bar@example.com", SMF_EMAIL_ADDRESS_TYPE_CC) == 0);
    fail_unless(smf_message_add_recipient(msg, "blubb@example.com", SMF_EMAIL_ADDRESS_TYPE_BCC) == 0);
    fail_unless(smf_message_add_recipient(msg, "moeeeeb@example.com", SMF_EMAIL_ADDRESS_TYPE_FROM) == 0);
    
    fail_unless((l = smf_message_get_recipients(msg)) != NULL);
    ck_assert_int_eq(smf_list_size(l), 3);
    e = smf_list_head(l);
    ck_assert_str_eq(smf_email_address_get_email((SMFEmailAddress_T*)e->data), "foo@example.com");
    ck_assert_int_eq(smf_email_address_get_type((SMFEmailAddress_T*)e->data), SMF_EMAIL_ADDRESS_TYPE_TO);
    e = smf_list_next(e);
    ck_assert_str_eq(smf_email_address_get_email((SMFEmailAddress_T*)e->data), "bar@example.com");
    ck_assert_int_eq(smf_email_address_get_type((SMFEmailAddress_T*)e->data), SMF_EMAIL_ADDRESS_TYPE_CC);
    e = smf_list_next(e);
    ck_assert_str_eq(smf_email_address_get_email((SMFEmailAddress_T*)e->data), "blubb@example.com");
    ck_assert_int_eq(smf_email_address_get_type((SMFEmailAddress_T*)e->data), SMF_EMAIL_ADDRESS_TYPE_BCC);
}
END_TEST

START_TEST(set_content_type) {
    smf_message_set_content_type(msg, "text/plain; charset=us-ascii");
    ck_assert_str_eq(smf_message_get_content_type(msg), "text/plain; charset=us-ascii");
}
END_TEST

START_TEST(set_content_transfer_encoding) {
    smf_message_set_content_transfer_encoding(msg, "multipart/mixed");
    ck_assert_str_eq(smf_message_get_content_transfer_encoding(msg), "multipart/mixed");
}
END_TEST

START_TEST(set_content_id) {
    char *mid;
    
    fail_unless((mid = smf_message_generate_message_id()) != NULL);
    smf_message_set_content_id(msg, mid);
    ck_assert_str_eq(smf_message_get_content_id(msg), mid);
    free(mid);
}
END_TEST

START_TEST(set_mime_version) {
    smf_message_set_mime_version(msg, "1.0");
    ck_assert_str_eq(smf_message_get_mime_version(msg), "1.0");
}
END_TEST

START_TEST(set_date_now) {
    fail_unless(smf_message_get_date(msg) == NULL);
    fail_unless(smf_message_set_date_now(msg) == 0);
    fail_unless(smf_message_get_date(msg) != NULL);
}
END_TEST

START_TEST(set_subject) {
    smf_message_set_subject(msg, "Holla die Waldfee");
    ck_assert_str_eq(smf_message_get_subject(msg), "Holla die Waldfee");
}
END_TEST

START_TEST(prepend_subject) {
    smf_message_set_subject(msg, "Holla die Waldfee");
    smf_message_prepend_subject(msg, "Re:");
    ck_assert_str_eq(smf_message_get_subject(msg), "Re: Holla die Waldfee");
}
END_TEST

START_TEST(append_subject) {
    smf_message_set_subject(msg, "Holla die Waldfee");
    smf_message_append_subject(msg, "2.0");
    ck_assert_str_eq(smf_message_get_subject(msg), "Holla die Waldfee 2.0");
}
END_TEST

START_TEST(set_body) {
    SMFPart_T *part;

    fail_unless(smf_message_set_body(msg, "schnickschnack") == 0);
    ck_assert_int_eq(smf_message_get_part_count(msg), 1);
    fail_unless((part = smf_list_head(msg->parts)->data) != NULL);
    ck_assert_str_eq(part->content, "schnickschnack");
}
END_TEST

START_TEST(generate_boundary) {
    char *b;
    
    fail_unless((b = smf_message_generate_boundary()) != NULL);
    smf_message_set_boundary(msg, b);
    ck_assert_str_eq(smf_message_get_boundary(msg), b);
    free(b);
}
END_TEST

START_TEST(append_part) {
    SMFPart_T *part;
    
    fail_unless(smf_message_set_body(msg, "schnickschnack") == 0);
    fail_unless((part = smf_part_new()) != NULL);
    fail_unless(smf_part_from_file(&part, SAMPLES_DIR "/redball.png", NULL) == 0);
    fail_unless(smf_message_append_part(msg, part) == 0);
    ck_assert_int_eq(smf_message_get_part_count(msg), 2);
}
END_TEST

START_TEST(add_attachment) {
    fail_unless(smf_message_set_body(msg, "schnickschnack") == 0);
    smf_message_add_attachment(msg, SAMPLES_DIR "/redball.png");
    ck_assert_int_eq(smf_message_get_part_count(msg), 2);
}
END_TEST

START_TEST(create_skeleton) {
    SMFMessage_T *m;
    SMFEmailAddress_T *addr;
    SMFList_T *l;

    fail_unless((m = smf_message_create_skeleton("sender", "recipient", "subject")) != NULL);
    fail_unless((addr = smf_message_get_sender(m)) != NULL);
    ck_assert_str_eq(smf_email_address_get_email(addr), "sender");
    fail_unless((l = smf_message_get_recipients(m)) != NULL);
    ck_assert_int_eq(smf_list_size(l), 1);
    fail_unless((addr = smf_list_head(l)->data) != NULL);
    ck_assert_str_eq(smf_email_address_get_email(addr), "recipient");
    ck_assert_str_eq(smf_message_get_subject(m), "subject");
    smf_message_free(m);
}
END_TEST

START_TEST(from_file) {
    char pathname[PATH_MAX];
    FILE *fh;
    long fsize;
    char *content, *dump;
    
    snprintf(pathname, sizeof(pathname), "%s/%s", SAMPLES_DIR, test_files[_i]);
    fail_unless(smf_message_from_file(&msg, pathname, 0) == 0);

    fail_unless((fh = fopen(pathname, "r")) != NULL);
    fail_unless(fseek(fh, 0, SEEK_END) == 0);
    fail_if((fsize = ftell(fh)) == -1);
    rewind(fh);
    
    content = malloc(fsize + 1);
    fail_unless(fread(content, 1, fsize, fh) == fsize);
    content[fsize] = '\0';
    
    fail_unless((dump = smf_message_to_string(msg)) != NULL);
    ck_assert_str_eq(content, dump);
    
    free(content);
    free(dump);
    fclose(fh);
}
END_TEST

START_TEST(to_fd) {
    char pathname[PATH_MAX];
    char tmp_fn[PATH_MAX];
    int fd;
    FILE *fh;
    long fsize;
    char *content, *dump;

    snprintf(pathname, sizeof(pathname), "%s/%s", SAMPLES_DIR, test_files[_i]);
    snprintf(tmp_fn, sizeof(tmp_fn), "%s.XXXXXX", __FUNCTION__);
    fail_if((fd = mkstemp(tmp_fn)) == -1);
    fail_unless(smf_message_to_fd(msg, fd) > 0);
    close(fd);
    
    fail_unless((fh = fopen(tmp_fn, "r")) != NULL);
    fail_unless(fseek(fh, 0, SEEK_END) == 0);
    fail_if((fsize = ftell(fh)) == -1);
    rewind(fh);
    content = malloc(fsize + 1);
    fail_unless(fread(content, 1, fsize, fh) == fsize);
    content[fsize] = '\0';
    fclose(fh);
    
    fail_unless((dump = smf_message_to_string(msg)) != NULL);
    ck_assert_str_eq(content, dump);
    
    fail_unless(unlink(tmp_fn) == 0);
}
END_TEST

TCase *messages_tcase() {
    TCase* tc = tcase_create("message");

    tcase_add_checked_fixture(tc, setup, teardown);
    tcase_add_test(tc, set_sender);
    tcase_add_test(tc, get_sender_string);
    tcase_add_test(tc, generate_message_id);
    tcase_add_test(tc, set_message_id);
    tcase_add_test(tc, set_header);
    tcase_add_test(tc, remove_header);
    tcase_add_test(tc, add_recipient);
    tcase_add_test(tc, set_content_type);
    tcase_add_test(tc, set_content_transfer_encoding);
    tcase_add_test(tc, set_content_id);
    tcase_add_test(tc, set_mime_version);
    tcase_add_test(tc, set_date_now);
    tcase_add_test(tc, set_subject);
    tcase_add_test(tc, prepend_subject);
    tcase_add_test(tc, append_subject);
    tcase_add_test(tc, set_body);
    tcase_add_test(tc, generate_boundary);
    tcase_add_test(tc, append_part);
    tcase_add_test(tc, add_attachment);
    tcase_add_test(tc, create_skeleton);
    tcase_add_loop_test(tc, from_file, 0, 54);
    tcase_add_loop_test(tc, to_fd, 0, 54);

    return tc;
}
