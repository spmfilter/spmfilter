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
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#include "test.h"

struct cb_data {
    int count;
    int rc;  
};

static SMFSettings_T *settings;
static SMFSession_T *session;
static SMFProcessQueue_T *queue;
static struct cb_data mod1_data;
static struct cb_data mod2_data;
static struct cb_data mod3_data;
static struct cb_data nexthop_data;
static struct cb_data error_data;
static struct cb_data processing_error_data;
static struct cb_data nexthop_error_data;

static void init_cb_data(struct cb_data* data) {
    data->count = 0;
    data->rc = 0;
}

static SMFMessage_T *load_sample_message(const char *sample) {
    SMFMessage_T *message;
    
    fail_unless((message = smf_message_new()) != NULL);
    fail_unless(smf_message_from_file(&message, sample, 1) == 0);
    
    return message;
}

static int mod1(SMFSession_T *s) {
    fail_unless(session == s);
    mod1_data.count++;
    return mod1_data.rc;
}

static int mod2(SMFSession_T *s) {
    fail_unless(session == s);
    mod2_data.count++;
    return mod2_data.rc;
}

static int mod3(SMFSession_T *s) {
    fail_unless(session == s);
    mod3_data.count++;
    return mod3_data.rc;
}

static int nexthop_cb(SMFSettings_T *set, SMFSession_T *ses) {
    fail_unless(settings == set);
    fail_unless(session == ses);
    nexthop_data.count++;
    return nexthop_data.rc;
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
        
    fail_unless((session = smf_session_new()) != NULL);
    
    queue = smf_modules_pqueue_init(nexthop_cb, error_cb, processing_error_cb, nexthop_error_cb);
    fail_unless(queue != NULL);
        
    init_cb_data(&mod1_data);
    init_cb_data(&mod2_data);
    init_cb_data(&mod3_data);
    init_cb_data(&nexthop_data);
    init_cb_data(&error_data);
    init_cb_data(&processing_error_data);
    init_cb_data(&nexthop_error_data);
}

static void teardown() {
    smf_settings_free(settings);
    smf_session_free(session);
    free(queue);
    session = NULL;
}

START_TEST(create_invoke_destroy) {
    SMFModule_T *module;
    
    fail_unless((module = smf_module_create(BINARY_DIR "/libtestmod1.so")) !=  NULL);
    fail_unless(smf_module_invoke(module, session) == 0);
    fail_unless(smf_module_destroy(module) == 0);
}
END_TEST

START_TEST(create_invoke_destroy_callback) {
    SMFModule_T *module;
    
    fail_unless(mod1_data.count == 0);
    fail_unless((module = smf_module_create_callback("foo", mod1)) != NULL);
    fail_unless(smf_module_invoke(module, session) == 0);
    fail_unless(smf_module_destroy(module) == 0);
    fail_unless(mod1_data.count == 1);
}
END_TEST

START_TEST(process_success) {
    session->envelope->message = load_sample_message(SAMPLES_DIR "/m0001.txt");
        
    smf_list_append(settings->modules, smf_module_create_callback("mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback("mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback("mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 1);
    fail_unless(nexthop_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 0);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_halt_queue) {
    session->envelope->message = load_sample_message(SAMPLES_DIR "/m0001.txt");
        
    smf_list_append(settings->modules, smf_module_create_callback("mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback("mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback("mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 0; // halt the queue, can be continued
        
    fail_unless(smf_modules_process(queue, session, settings) == -1);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(error_data.count == 0);
    fail_unless(nexthop_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
    
    // queue can be continued
    mod2_data.rc = 0;
    
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1); // Now skipped
    fail_unless(mod2_data.count == 2); // Repeated, but now successful
    fail_unless(mod3_data.count == 1); // Now executed
    fail_unless(nexthop_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_stop_queue) {
    session->envelope->message = load_sample_message(SAMPLES_DIR "/m0001.txt");
        
    smf_list_append(settings->modules, smf_module_create_callback("mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback("mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback("mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 1; // stop the queue
    
    fail_unless(smf_modules_process(queue, session, settings) == 1);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(nexthop_data.count == 0);
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
    fail_unless(nexthop_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_nexthop) {
    session->envelope->message = load_sample_message(SAMPLES_DIR "/m0001.txt");
        
    smf_list_append(settings->modules, smf_module_create_callback("mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback("mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback("mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    processing_error_data.rc = 2; // stop the queue, invoke nexthop
 
    fail_unless(smf_modules_process(queue, session, settings) == 0);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(nexthop_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 0);
}
END_TEST

START_TEST(process_err_nexthop_err) {
    session->envelope->message = load_sample_message(SAMPLES_DIR "/m0001.txt");
        
    smf_list_append(settings->modules, smf_module_create_callback("mod1", mod1));
    smf_list_append(settings->modules, smf_module_create_callback("mod2", mod2));
    smf_list_append(settings->modules, smf_module_create_callback("mod3", mod3));
    fail_unless(smf_list_size(settings->modules) == 3);
    
    mod2_data.rc = 1; // != 0 is that counts
    nexthop_data.rc = 4711; // != 0 is that counts
    processing_error_data.rc = 2; // stop the queue, invoke nexthop
 
    fail_unless(smf_modules_process(queue, session, settings) == 4711);
    fail_unless(mod1_data.count == 1);
    fail_unless(mod2_data.count == 1);
    fail_unless(mod3_data.count == 0);
    fail_unless(nexthop_data.count == 1);
    fail_unless(error_data.count == 0);
    fail_unless(processing_error_data.count == 1);
    fail_unless(nexthop_error_data.count == 1);
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
    
    return tc;
}
