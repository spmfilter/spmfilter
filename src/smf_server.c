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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_server.h"

#define THIS_MODULE "server"

void smf_server_init(SMFSettings_T *settings) {
    pid_t pid;
    FILE *pidfile;
    int i, sigs[] = { SIGHUP, SIGINT, SIGQUIT, SIGTSTP, SIGTTIN, SIGTTOU };
    struct sigaction action;

    action.sa_handler = SIG_IGN;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    for (i = 0; i < sizeof(sigs) / sizeof(int); i++) {
        if (sigaction(sigs[i],&action, NULL) < 0) {
            TRACE(TRACE_ERR,"sigaction failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    switch( pid = fork()) {
        case -1:
            TRACE(TRACE_ERR,"fork failed: %s",strerror(errno));
            break;
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
            break;
    }

    if (setsid() < 0) {
        TRACE(TRACE_ERR,"setsid failed: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    switch (pid = fork()) {
        case -1:
            TRACE(TRACE_ERR,"fork failed: %s",strerror(errno));
            exit(EXIT_FAILURE);
            break;
        case 0:
            break;
        default:
            exit(EXIT_SUCCESS);
            break;
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    chdir(settings->queue_dir);
    umask(0);

    if( settings->pid_file != NULL ) {
        pidfile = fopen(settings->pid_file, "w+" );
        if( pidfile == NULL ) {
            TRACE(TRACE_ERR, "can't open PID file %s: %s",settings->pid_file, strerror(errno));
        } else {
            fprintf(pidfile, "%d\n", getpid());
            fclose(pidfile);
        }
    }
}

int smf_server_listen(SMFSettings_T *settings) {

}
