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

#include <stdio.h>

#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_modules.h"
#include "../src/smf_session.h"
#include "../src/smf_session_private.h"
#include "../src/smf_envelope.h"
#include "../src/smf_message.h"

#include "test.h"

static int _handle_q_error(SMFSettings_T *settings, SMFSession_T *session) {

    return 0;
}

static int _handle_q_processing_error(SMFSettings_T *settings, SMFSession_T *session, int retval) {

    return 0;
}

static int _handle_nexthop_error(SMFSettings_T *settings, SMFSession_T *session) {
    
    return 0;
}

int main(int argc, char const *argv[]) {
    SMFSettings_T *settings = smf_settings_new();
    SMFProcessQueue_T *q;
    SMFSession_T *session = smf_session_new();
    SMFMessage_T *message = smf_message_new();
    SMFEnvelope_T *env = smf_session_get_envelope(session);
    char *s;

    smf_settings_set_debug(settings, 1);
    smf_envelope_set_nexthop(env, "localhost:25");
    smf_envelope_set_sender(env, test_email);
    smf_envelope_add_rcpt(env, test_email);

    printf("Start smf_modules tests...\n");
    printf("============================================\n");
    printf("This test expects a running SMTP daemon, \n");
    printf("listening on localhost:25, wich accepts \n");
    printf("mails for user@example.org\n");
    printf("============================================\n");

    /* preparing session stuff */
    asprintf(&s, "%s/m0001.txt",SAMPLES_DIR);
    smf_message_from_file(&message, s, 0);
    free(s);
    session->envelope->message = message;

    smf_settings_set_queue_dir(settings, BINARY_DIR);
    smf_core_gen_queue_file(settings->queue_dir, &session->message_file, session->id);
    s = smf_message_generate_message_id();
    smf_message_set_message_id(message, s);
    free(s);
    smf_message_to_file(message, session->message_file);

    /* add test modules */
    smf_settings_add_module(settings, "testmod1");
    smf_settings_add_module(settings, "testmod2");

    printf("* testing smf_modules_init()...\t\t\t");
    if (smf_modules_init(settings,BINARY_DIR)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_modules_pqueue_init()...\t\t");
    q = smf_modules_pqueue_init(
        _handle_q_error,
        _handle_q_processing_error,
        _handle_nexthop_error
    );

    if(q == NULL) {
        printf("failed\n");
        return(-1);
    }
    printf("passed\n");

    printf("* testing smf_modules_process()...\t\t");
    if (smf_modules_process(q,session,settings)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    
    printf("* testing smf_modules_deliver_nexthop()...\t");
    if (smf_modules_deliver_nexthop(settings,q,session)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");


    free(q);
    smf_session_free(session);

    printf("* testing smf_modules_unload()...\t\t");
    if (smf_modules_unload(settings)!=0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    smf_settings_free(settings);

    return 0;
}
