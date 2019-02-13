/* spmfilter - mail filtering framework
 * Copyright (C) 2012-2016 Axel Steiner and SpaceNet AG
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

#include <sys/types.h>
#include <check.h>
#include <stdio.h>
#include <utime.h>

#include "../src/smf_modules.h"
#include "../src/smf_session.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#include "test.h"
#include "test_params.h"

struct cb_data {
    int count;
    int rc;  
};

static SMFSettings_T *settings;
static SMFSession_T *session;
static SMFProcessQueue_T *queue;
static char spoolfile[PATH_MAX];
static struct cb_data mod1_data;
static struct cb_data mod2_data;
static struct cb_data mod3_data;
static struct cb_data error_data;
static struct cb_data processing_error_data;
static struct cb_data nexthop_error_data;

static void init_cb_data(struct cb_data* data) {
    data->count = 0;
    data->rc = 0;
}

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
    
    snprintf(spoolfile, sizeof(spoolfile), "%s/XXXXXX", BINARY_DIR);
    fail_if((src = open(sample, O_RDONLY)) == -1);
    fail_if((dest = mkstemp(spoolfile)) == -1);
    
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
    smf_session_set_message_file(session, spoolfile);
    
    return spoolfile;
}

static int mod1(SMFSettings_T *set, SMFSession_T *s) {
    fail_unless(set == settings);
    fail_unless(session == s);
    mod1_data.count++;
    return mod1_data.rc;
}

static int mod2(SMFSettings_T *set,  SMFSession_T *s) {
    fail_unless(set == settings);
    fail_unless(session == s);
    mod2_data.count++;
    return mod2_data.rc;
}

static int mod3(SMFSettings_T *set, SMFSession_T *s) {
    fail_unless(set == settings);
    fail_unless(session == s);
    mod3_data.count++;
    return mod3_data.rc;
}

static int message_file_changed_cb(SMFSettings_T *set, SMFSession_T *s) {
  struct stat fstat;
  struct utimbuf times;

  fail_unless(stat(s->message_file, &fstat) == 0);
  times.actime = fstat.st_ctime + 10;
  times.modtime = fstat.st_mtime + 10;
  fail_unless(utime(s->message_file, &times) == 0);

  return 0;
}

static int error_cb(SMFSettings_T *set, SMFSession_T *ses) {
    fail_unless(settings == set);
    fail_unless(session == ses);
    error_data.count++;
    return error_data.rc;
}

static int processing_error_cb(SMFSettings_T *set, SMFSession_T *ses, int retval) {
    fail_unless(settings == set);
    fail_unless(session == ses);
    processing_error_data.count++;
    return processing_error_data.rc;
}

static int nexthop_error_cb(SMFSettings_T *set, SMFSession_T *ses) {
    fail_unless(settings == set);
    fail_unless(session == ses);
    nexthop_error_data.count++;
    return nexthop_error_data.rc;
}

static void setup() {
    fail_unless((settings = smf_settings_new()) != NULL);
    smf_settings_set_queue_dir(settings, BINARY_DIR);
    smf_settings_set_lib_dir(settings, BINARY_DIR);
    fail_unless((session = smf_session_new()) != NULL);
    
    queue = smf_modules_pqueue_init(error_cb, processing_error_cb, nexthop_error_cb);
    fail_unless(queue != NULL);
    
    create_sample_message(session, SAMPLES_DIR "/m0001.txt");
    create_spoolfile(session, SAMPLES_DIR "/m0001.txt");
        
    init_cb_data(&mod1_data);
    init_cb_data(&mod2_data);
    init_cb_data(&mod3_data);
    init_cb_data(&error_data);
    init_cb_data(&processing_error_data);
    init_cb_data(&nexthop_error_data);
}

static void teardown() {
    fail_unless(unlink(spoolfile) == 0);
    smf_settings_free(settings);
    smf_session_free(session);
    free(queue);
    session = NULL;
}

START_TEST(create_invoke_destroy) {
    SMFModule_T *module;

    fail_unless((module = smf_module_create(settings, "testmod1")) !=  NULL);
    fail_unless(smf_module_invoke(settings, module, session) == 0);
    fail_unless(smf_module_destroy(module) == 0);
}
END_TEST

START_TEST(create_invoke_destroy_callback) {
    SMFModule_T *module;
    
    fail_unless(mod1_data.count == 0);
    fail_unless((module = smf_module_create_callback(settings, "foo", mod1)) != NULL);
    fail_unless(smf_module_invoke(settings, module, session) == 0);
    fail_unless(smf_module_destroy(module) == 0);
    fail_unless(mod1_data.count == 1);
}
END_TEST

START_TEST(process_success) {
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 0);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_halt_queue) {
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 0; // halt the queue, can be continued
        
    fail_unless(smf_modules_process(queue, session, settings) == -1);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
    
    // queue can be continued
    mod2_data.rc = 0;
    
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1); // Now skipped
    fail_unless(mod2_data.count == 2); // Repeated, but now successful
    fail_unless(mod3_data.count == 1); // Now executed
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_stop_queue) {
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 1; // stop the queue
    
    fail_unless(smf_modules_process(queue, session, settings) == 1);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
    
    // processing the module-queue again will invoke all modules
    mod2_data.rc = 0;
    processing_error_data.rc = 0;
    
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 2);
    fail_unless(mod2_data.count == 2);
    fail_unless(mod3_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_nexthop) {
    smf_settings_set_nexthop(settings, "/dev/null");
        
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 2; // stop the queue, invoke nexthop
 
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_nexthop_err) {
    smf_settings_set_nexthop(settings, "something_that_not_exists");
        
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback(settings, "mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
        
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 2; // stop the queue, invoke nexthop
     
    fail_unless(smf_modules_process(queue, session, settings) == -1);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 1);
}
END_TEST

START_TEST(message_file_changed) {
  SMFModule_T *module;
  SMFMessage_T *old_msg_ptr;

  fail_unless((old_msg_ptr = session->envelope->message) != NULL);
  fail_unless((module = smf_module_create_callback(settings, "foo", message_file_changed_cb)) != NULL);
  fail_unless(smf_module_invoke(settings, module, session) == 0);
  fail_unless(smf_module_destroy(module) == 0);
  fail_unless(old_msg_ptr != session->envelope->message); /* Reloaded */
}
END_TEST

TCase *modules_tcase() {
    TCase* tc = tcase_create("modules");
    tcase_add_checked_fixture(tc, setup, teardown);
    
    tcase_add_test(tc, create_invoke_destroy);
    tcase_add_test(tc, create_invoke_destroy_callback);
    tcase_add_test(tc, process_success);
    tcase_add_test(tc, process_err_halt_queue);
    tcase_add_test(tc, process_err_stop_queue);
    tcase_add_test(tc, process_err_nexthop);
    tcase_add_test(tc, process_err_nexthop_err);
    tcase_add_test(tc, message_file_changed);
    
    return tc;
}
