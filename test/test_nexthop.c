/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner, Sebastian Jäkel and SpaceNet AG
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

#include <sys/stat.h>
#include <check.h>
#include <stdio.h>

#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_session.h"
#include "../src/smf_nexthop.h"

#include "testdirs.h"

static SMFSettings_T *settings;
static SMFSession_T *session;
static char spool_file[PATH_MAX];
static char dest_file[PATH_MAX];

static SMFMessage_T *create_sample_message(SMFSession_T *session, const char *sample) {
    SMFMessage_T *message;
    
    fail_unless((message = smf_message_new()) != NULL);
    fail_unless(smf_message_from_file(&message, sample, 1) == 0);
    session->envelope->message = message;
    
    return message;
}

static const char *create_spoolfile(SMFSession_T *session, const char *sample) {
    int src, dest;
    char buf[512];
    
    snprintf(spool_file, sizeof(spool_file), "%s/XXXXXX", BINARY_DIR);
    fail_if((src = open(sample, O_RDONLY)) == -1);
    fail_if((dest = mkstemp(spool_file)) == -1);
    
    while (1) {
        size_t nread, nwritten;
        
        if ((nread = read(src, buf, sizeof(buf))) == 0)
            break;
        fail_unless(nread > 0);
        
        nwritten = write(dest, buf, nread);
        fail_unless(nread == nwritten);
    }
    
    close(src);
    close(dest);
    smf_session_set_message_file(session, spool_file);
    
    return spool_file;
}

static void setup() {
    int fh;
    
    fail_unless((settings = smf_settings_new()) != NULL);
    fail_unless((session = smf_session_new()) != NULL);
    
    snprintf(dest_file, sizeof(dest_file), "%s/XXXXXX", BINARY_DIR);
    fail_if((fh = mkstemp(dest_file)) == -1);
    close(fh);
    
    create_sample_message(session, SAMPLES_DIR "/m0001.txt");
    create_spoolfile(session, SAMPLES_DIR "/m0001.txt");
}

static void teardown() {
    fail_if(unlink(spool_file) == -1);
    fail_if(unlink(dest_file) == -1);
    smf_settings_free(settings);
    smf_session_free(session);
}

START_TEST(file_no_src) {
    NexthopFunction func;

    smf_settings_set_nexthop(settings, dest_file);
    fail_if(chmod(spool_file, S_IWUSR) == -1);
    
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == -1);
}
END_TEST

START_TEST(file_no_dest) {
    NexthopFunction func;

    smf_settings_set_nexthop(settings, dest_file);
    fail_if(chmod(dest_file, S_IRUSR) == -1);
    
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == -1);
}
END_TEST

START_TEST(file_success) {
    NexthopFunction func;

    smf_settings_set_nexthop(settings, dest_file);
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == 0);
}
END_TEST

START_TEST(smtp_no_src) {
    NexthopFunction func;
    
    fail_if(chmod(spool_file, S_IWUSR) == -1);
    smf_settings_set_nexthop(settings, "localhost:25");
    
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == -1);
}
END_TEST

START_TEST(smtp_no_dest) {
    NexthopFunction func;
    
    smf_settings_set_nexthop(settings, "a_non_existing_host");
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == -1);
}
END_TEST

START_TEST(smtp_success) {
    NexthopFunction func;
    
    smf_settings_set_nexthop(settings, "localhost:33332");
    
    fail_unless((func = smf_nexthop_find(settings)) != NULL);
    fail_unless(func(settings, session) == 0);
}
END_TEST

TCase *nexthop_tcase() {
    TCase *tc = tcase_create("modules");
    tcase_add_checked_fixture(tc, setup, teardown);
    
    tcase_add_test(tc, file_no_src);
    tcase_add_test(tc, file_no_dest);
    tcase_add_test(tc, file_success);
    tcase_add_test(tc, smtp_no_src);
    tcase_add_test(tc, smtp_no_dest);
    tcase_add_test(tc, smtp_success);
    
    return tc;
}
