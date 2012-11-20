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
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>

#include "../src/smf_smtp.h"
#include "../src/smf_envelope.h"
#include "../src/smf_settings.h"
#include "test.h"
#include "testdirs.h"

int main (int argc, char const *argv[]) {
    SMFSmtpStatus_T *status = NULL;
    SMFEnvelope_T *env = smf_envelope_new();
    SMFMessage_T *msg = NULL;
    char *msg_file = NULL;

    printf("Start smf_smtp tests...\n");
    printf("============================================\n");
    printf("This test expects a running SMTP daemon, \n");
    printf("listening on localhost:25, wich accepts \n");
    printf("mails for user@example.org\n");
    printf("============================================\n");

    printf("* testing smf_smtp_status_new()...\t\t\t\t");
    status = smf_smtp_status_new();
    if (status == NULL) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_smtp_status_free()...\t\t\t\t");
    status->text = strdup(test_string);
    smf_smtp_status_free(status);
    printf("passed\n");

    asprintf(&msg_file, "%s/m0001.txt",SAMPLES_DIR);

    printf("* testing smf_smtp_deliver() with file...\t\t\t");
    smf_envelope_set_nexthop(env, "localhost:25");
    smf_envelope_set_sender(env, test_email);
    smf_envelope_add_rcpt(env, test_email);
    
    status = smf_smtp_deliver(env, SMF_TLS_DISABLED, msg_file, NULL);
    if (status->code == -1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    smf_smtp_status_free(status);
    printf("* testing smf_smtp_deliver() with envelope...\t\t\t");
    msg = smf_message_new();
    if (smf_message_from_file(&msg, msg_file, 0) != 0){
        printf("failed\n");
        return -1;
    }
    smf_envelope_set_message(env, msg);
    status = smf_smtp_deliver(env, SMF_TLS_DISABLED, NULL, NULL);
    if (status->code == -1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    smf_smtp_status_free(status);

    printf("* testing smf_smtp_deliver() with envelope and TLS...\t\t");
    status = smf_smtp_deliver(env, SMF_TLS_REQUIRED, NULL, NULL);
    if (status->code == -1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    smf_smtp_status_free(status);

    printf("* testing smf_smtp_deliver() with smtp auth...\t\t\t");
    smf_envelope_set_auth_user(env, test_auth_user);
    smf_envelope_set_auth_pass(env, test_auth_pass);
    status = smf_smtp_deliver(env, SMF_TLS_DISABLED, NULL, NULL);
    if (status->code == -1) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");
    smf_smtp_status_free(status);

    free(msg_file);
    smf_envelope_free(env);
    return 0;
}
