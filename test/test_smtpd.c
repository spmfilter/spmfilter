/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2019 Axel Steiner, Werner Detter and SpaceNet AG
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
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <syslog.h>

#include "test.h"
#include "test_params.h"
#include "../src/smf_internal.h"
#include "../src/smf_server.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_smtp.h"
#include "../src/smf_envelope.h"

int main (int argc, char const *argv[]) {
    char *msg_file = NULL;
    SMFSmtpStatus_T *status = NULL;
    SMFSettings_T *settings = smf_settings_new();
    SMFEnvelope_T *env = smf_envelope_new();
    pid_t pid;
    char *delivery_destination = NULL;

    printf("Start smf_smtpd tests...\n");

    printf("* preparing smtpd engine...\t\t\t");

    if (smf_settings_parse_config(&settings,"samples/spmfilter.conf") != 0) {
        printf("failed\n");
        return -1;
    }
    
    printf("passed\n");
    switch(pid = fork()) {
        case -1:
            printf("failed\n");
            break;
        case 0:
            if (smf_modules_engine_load(settings) != 0) {
                return -1;
            }

            break;

        default:
            sleep (1);
            printf("* sending test message ...\t\t\t");
            asprintf(&msg_file, "%s/m2004.txt",SAMPLES_DIR);

            asprintf(&delivery_destination, "%s:%d",smf_settings_get_bind_ip(settings), smf_settings_get_bind_port(settings));
            smf_envelope_set_nexthop(env, delivery_destination);
            smf_envelope_set_sender(env, test_email);
            smf_envelope_add_rcpt(env, test_email);
            status = smf_smtp_deliver(env, SMF_TLS_DISABLED, msg_file, NULL);
            free(delivery_destination);
            if (status->code != 250) {
                kill(pid,SIGTERM);
                printf("failed\n");
                return -1;
            }

            smf_smtp_status_free(status);
            printf("passed\n");

            kill(pid,SIGTERM);
            waitpid(pid, NULL, 0);

            break;
    }

    if(msg_file != NULL)
        free(msg_file);

    smf_settings_free(settings);
    smf_envelope_free(env);

    return 0;
}


