/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner, Werner Detter and SpaceNet AG
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


#include "test.h"
#include "../src/smf_internal.h"
#include "../src/smf_server.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_smtp.h"
#include "../src/smf_envelope.h"

int load(SMFSettings_T *settings);

int main (int argc, char const *argv[]) {
    char *msg_file = NULL;

    SMFSmtpStatus_T *status = NULL;
    SMFSettings_T *settings = smf_settings_new();
    SMFEnvelope_T *env = smf_envelope_new();
    pid_t pid;

    smf_settings_set_pid_file(settings, "/tmp/smf_test_smtpd.pid");
    smf_settings_set_bind_ip(settings, "127.0.0.1");
    smf_settings_set_foreground(settings, 1);
    smf_settings_set_spare_childs(settings, 0);
    smf_settings_set_max_childs(settings,1);
    smf_settings_set_debug(settings,1);
    smf_settings_set_queue_dir(settings, BINARY_DIR);
    
    printf("Start smf_smtpd tests...\n");
    printf("* forking up smtpd()...\t\t\t");
    
    switch(pid = fork()) {
        case -1:
            printf("fork() failed: %s\n",strerror(errno));
            break;
        case 0:
            if(load(settings) != 0) {
                return -1;
            }
            break;

        default:
            sleep(10);
            printf("* sending test message ...\t\t");
            asprintf(&msg_file, "%s/m0001.txt",SAMPLES_DIR);

            smf_envelope_set_nexthop(env, "127.0.0.1:10025");
            smf_envelope_set_sender(env, test_email);
            smf_envelope_add_rcpt(env, test_email);
            status = smf_smtp_deliver(env, SMF_TLS_DISABLED, msg_file, NULL);
            
            if (status->code == -1) {
                kill(pid,SIGTERM);
                printf("failed\n");
                return -1;
            }
            
            smf_smtp_status_free(status);
            printf("passed\n");
            kill(pid,SIGTERM);
            break;
    }

    if(msg_file != NULL)
        free(msg_file);
        
    
    smf_settings_free(settings);
    smf_envelope_free(env);

    return 0;
}


