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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <assert.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>

#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_server.h"
#include "smf_modules.h"
#include "smf_settings_private.h"

#define THIS_MODULE "server"

int num_procs = 0;
int num_clients = 0;
int num_spare = 0;
int daemon_exit = 0;
int child[] = {};

void smf_server_sig_handler(int sig) {
    /**
     * - SIGUSR1 => child got a new client
     * - SIGUSR2 => child client closes connections
     */
    switch(sig) {
        case SIGTERM:
        case SIGINT:
            daemon_exit = 1;
            break;
        case SIGUSR1:
            num_clients++;
            num_spare--;
            break;
        case SIGUSR2:
            num_clients--;
            break;
        default:
            break;
    }

    return;
}

void smf_server_sig_init(void) {
    struct sigaction action, old_action;

    action.sa_handler = smf_server_sig_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGTERM, &action, &old_action) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGTERM) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGINT, &action, &old_action) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGINT) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGUSR1, &action, &old_action) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGUSR1) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (sigaction(SIGUSR2, &action, &old_action) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGUSR2) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }

}

void smf_server_init(SMFSettings_T *settings, int sd) {
    pid_t pid;
    FILE *pidfile;
    
    struct passwd *pwd = NULL;
    struct group *grp = NULL;
   
    smf_server_sig_init();

    /* switch to background */
    if (settings->foreground == 0) {        
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
    }

    /* switch user */
    if ((settings->user != NULL) && (settings->group != NULL)) {
        TRACE(TRACE_DEBUG,"switching to user %s:%s",settings->user,settings->group);
        grp = getgrnam(settings->group);

        if (grp == NULL) {
            TRACE(TRACE_ERR, "could not find group %s", settings->group);
            exit(EXIT_FAILURE);
        }

        pwd = getpwnam(settings->user);
        if (pwd == NULL) {
            TRACE(TRACE_ERR, "could not find user %s", settings->user);
            exit(EXIT_FAILURE);
        }

        if (setgid(grp->gr_gid) != 0) {
            TRACE(TRACE_ERR, "could not set gid to %s", settings->group);
            exit(EXIT_FAILURE);
        }

        if (setuid(pwd->pw_uid) != 0) {
            TRACE(TRACE_ERR, "could not set uid to %s", settings->user);
            exit(EXIT_FAILURE);
        }
    }

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
    int sd, reuseaddr;
    int status = -1;
    struct addrinfo hints, *ai, *aptr;
    char *srvname = NULL;

    assert(settings);

    memset(&hints,0,sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    TRACE(TRACE_INFO,"binding to %s:%d",settings->bind_ip,settings->bind_port);

    asprintf(&srvname,"%d",settings->bind_port);
    if ((status == getaddrinfo(settings->bind_ip,srvname,&hints,&ai)) == 0) {
        for (aptr = ai; aptr != NULL; aptr = aptr->ai_next) {
            if ((sd = socket(aptr->ai_family,aptr->ai_socktype, aptr->ai_protocol)) < 0)
                continue;

            reuseaddr = 1;
            setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

            if (bind(sd,aptr->ai_addr,aptr->ai_addrlen) == 0) {
                if (listen(sd, settings->listen_backlog) >= 0)
                    break;
            }
            close(sd);
        }

        freeaddrinfo(ai);

        if (aptr == NULL) {
            TRACE(TRACE_ERR,"can't listen on port %s: %s", srvname, strerror(errno));
            return -1;
        }
    } else {
        TRACE(TRACE_ERR,"getaddrinfo failed: %s",gai_strerror(status));
        return -1;
    }

    free(srvname);

    return sd;
}

void smf_server_fork(SMFSettings_T *settings,int sd, SMFProcessQueue_T *q,
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q)) {
    int pos = 0;

    for (pos=0; pos < settings->max_childs; pos++) {
        if (child[pos] == 0) {
            break;
        }
    }

    switch(child[pos] = fork()) {
        case -1:
            TRACE(TRACE_ERR,"fork() failed: %s",strerror(errno));
            break;
        case 0:

            smf_server_accept_handler(settings,sd,q,handle_client_func);
            
            exit(EXIT_SUCCESS); /* quit child process */
            break;
        default: /* parent process: go on with accept */
            TRACE(TRACE_DEBUG,"forked child [%d]\n",child[pos]);
            break;
    }
    num_procs++;
}

void smf_server_loop(SMFSettings_T *settings,int sd, SMFProcessQueue_T *q,
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q)) {
    int i, status;
    pid_t pid;

    TRACE(TRACE_NOTICE, "smf_server is coming up");

    for(i=0; i<settings->max_childs; i++)
        child[i] = 0;

    /* prefork min. 1 child(s) */
    if(settings->spare_childs == 0) {
        smf_server_fork(settings,sd,q,handle_client_func);
    } else {
        for (i = 0; i < settings->spare_childs; i++) {
            num_spare++;
            smf_server_fork(settings,sd,q,handle_client_func);
        }
    }

    for (;;) {
        pid = waitpid(-1, &status, 0);
        if (pid > 0) {

            for (i=0; i < settings->max_childs; i++) {
                if (pid == child[i]) {
                    child[i] = 0; /* remove process id */
                    num_procs--;
                    break;
                }
            }
        }

        if (num_procs < settings->max_childs) {
            /* minimal number of childs is not running */
            while (num_spare < settings->spare_childs) {
                smf_server_fork(settings,sd,q,handle_client_func);
                num_spare++;
            }      
        }

        if (daemon_exit)
            break;
    }

    TRACE(TRACE_NOTICE, "smf_server is going down");
	
    close(sd);

    for (i = 0; i < settings->max_childs; i++)
        if (child[i] > 0)
            kill(child[i],SIGTERM);
    while(wait(NULL) > 0)
        ;

    unlink(settings->pid_file);
}

void smf_server_accept_handler(SMFSettings_T *settings, int sd, SMFProcessQueue_T *q, 
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q)) {
    int client;
    socklen_t slen;
    struct sockaddr_storage sa;

    /* process incoming connection in an infinite loop */
    for (;;) {
        slen = sizeof(sa);

        /* accept new connection */
        if ((client = accept(sd, (struct sockaddr *)&sa, &slen)) < 0) {
            if (daemon_exit)
                break;

            if (errno != EINTR) {
                TRACE(TRACE_ERR,"accept failed: %s",strerror(errno));
            }
            continue;
        }
        handle_client_func(settings,client,q);
        close(client);
    }

}

