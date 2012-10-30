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
    SMFEnvelope_T *envelope = smf_envelope_new();
    SMFMessage_T *message = smf_message_new();
    int ret;
    char *s;

    printf("Start smf_modules tests...\n");

    envelope->message = message;
    session->envelope = envelope;

    s = smf_message_generate_message_id();
    smf_message_set_message_id(message, s);
    free(s);

    smf_settings_add_module(settings, "testmod1");
    smf_settings_add_module(settings, "testmod2");

    smf_modules_init(settings,BINARY_DIR);

    q = smf_modules_pqueue_init(
        _handle_q_error,
        _handle_q_processing_error,
        _handle_nexthop_error
    );

    if(q == NULL) {
        return(-1);
    }

    ret = smf_modules_process(q,session,settings);
    free(q);

    smf_modules_unload(settings);

    smf_settings_free(settings);

    return 0;
}
