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
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <pwd.h>
#include <grp.h>

#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_server.h"
#include "smf_modules.h"
#include "smf_settings_private.h"

#define THIS_MODULE "server"

#define SEM_LOCK -1
#define SEM_UNLOCK 1

static volatile int daemon_exit = 0;
static struct sembuf semaphore;

int _smf_server_init_ipc(SMFSettings_T *settings, SMFServerState_T *state) {
    SMFServerCounters_T *counters;
    int size_mem;
    int size_max_childs;
    int i;
    state->sem_key = ftok(".", 's');
    state->sem_id = semget(state->sem_key, 0, IPC_PRIVATE);
    if (state->sem_id < 0) {
        state->sem_id = semget(state->sem_key, 1, IPC_CREAT | IPC_EXCL | 0600);
        if (state->sem_id < 0) {
            TRACE(TRACE_ERR,"failed to create semaphore: %s", strerror(errno));
            return -1;
        }
        TRACE(TRACE_DEBUG,"created semaphore: %d\n", state->sem_id);
        if (semctl(state->sem_id, 0, SETVAL, (int) 1) == -1) {
            TRACE(TRACE_DEBUG,"failed to initialize semaphore: %s", strerror(errno));
            return -1;
        }
    }

    state->shm_key = ftok(".",'a');
    size_max_childs = 2 * (CHILD_LIMIT * sizeof(int));
    size_mem = sizeof(SMFServerCounters_T) + size_max_childs;

    state->shm_id = shmget(state->shm_key,size_mem,0600 | IPC_CREAT);
    if (state->shm_id < 0) {
        TRACE(TRACE_ERR,"failed to create shm segment: %s", strerror(errno));
        return -1;
    }

    
    TRACE(TRACE_DEBUG,"created shm segment: %d",state->shm_id);
    counters = (SMFServerCounters_T *)shmat(state->shm_id, (void *)0, 0);
    if (counters == (void *)(-1)) {
        TRACE(TRACE_ERR,"failed to attach shm segment: %s", strerror(errno));
        return -1;
    }

    counters->num_procs = 0;
    counters->num_spare = 0;
    counters->max_childs = settings->max_childs;
    
    for (i=0; i < settings->max_childs; i++ ) {
        counters->childs_active[i] = 0;
        counters->childs[i] = 0;
    }

    if (shmdt(counters) == -1) {
        TRACE(TRACE_ERR,"failed to detach shm segment: %s", strerror(errno));
        return -1;
    }

    return 0;
}

void _smf_server_sem_operation(int op, int sem_id) {
    semaphore.sem_op = op;
    semaphore.sem_flg = SEM_UNDO;
    
    if( semop(sem_id, &semaphore, 1) == -1) {
        TRACE(TRACE_ERR, "semop failed: %s", strerror(errno));
        exit (EXIT_FAILURE);
    }
}

SMFServerCounters_T *_smf_server_get_counters(int shm_id) {
    SMFServerCounters_T *counters;
    counters = (SMFServerCounters_T *)shmat(shm_id, (void *)0, 0);
    if (counters == (void *)(-1)) {
        TRACE(TRACE_ERR,"failed to attach shm segment: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return counters;
}

void _smf_server_detach_counters(SMFServerCounters_T *counters) {
    if (shmdt(counters) == -1) {
        TRACE(TRACE_ERR,"failed to detach shm segment: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void _smf_server_add_child(SMFServerState_T *state, pid_t pid) {
    SMFServerCounters_T *counters = _smf_server_get_counters(state->shm_id);
    int i;
    _smf_server_sem_operation(SEM_LOCK,state->sem_id);

    counters->num_procs++;
    counters->num_spare++;
    
    for(i=0; i<counters->max_childs; i++ ) {
        if (counters->childs[i] == 0) {
            counters->childs[i] = pid;
            break;
        }
    }

    _smf_server_detach_counters(counters);
    _smf_server_sem_operation(SEM_UNLOCK,state->sem_id);
}

void smf_server_decrement_spare(SMFServerState_T *state) {
    SMFServerCounters_T *counters = _smf_server_get_counters(state->shm_id);
    _smf_server_sem_operation(SEM_LOCK,state->sem_id);

    counters->num_spare--;
    
    _smf_server_detach_counters(counters);
    _smf_server_sem_operation(SEM_UNLOCK,state->sem_id);
}

void _smf_server_decrement_proc(SMFServerState_T *state) {
    SMFServerCounters_T *counters = _smf_server_get_counters(state->shm_id);
    _smf_server_sem_operation(SEM_LOCK,state->sem_id);

    counters->num_procs--;
    
    _smf_server_detach_counters(counters);
    _smf_server_sem_operation(SEM_UNLOCK,state->sem_id);
}

void smf_server_add_active(SMFServerState_T *state,int pid) {
    SMFServerCounters_T *counters = _smf_server_get_counters(state->shm_id);
    int i;
    _smf_server_sem_operation(SEM_LOCK,state->sem_id);

    for(i=0; i<counters->max_childs; i++ ) {
        if (counters->childs_active[i] == 0) {
            counters->childs_active[i] = pid;
            break;
        }
    }

    _smf_server_detach_counters(counters);
    _smf_server_sem_operation(SEM_UNLOCK,state->sem_id);
}

void _smf_server_remove_active(SMFServerState_T *state,int pid) {
    SMFServerCounters_T *counters = _smf_server_get_counters(state->shm_id);
    int i;
    int removed = -1;

    _smf_server_sem_operation(SEM_LOCK,state->sem_id);
    for(i=0; i<counters->max_childs; i++ ) {
        if (counters->childs_active[i] == pid) {
            counters->childs_active[i] = 0;
            removed = 1;
            break;
        }
    }

    for(i=0; i<counters->max_childs; i++ ) {
        if (counters->childs[i] == pid) {
            counters->childs[i] = 0;
            break;
        }
    }

    _smf_server_detach_counters(counters);
    _smf_server_sem_operation(SEM_UNLOCK,state->sem_id);

    /* it seems we lost a spare child */
    if (removed != 1) {
        TRACE(TRACE_ERR,"lost spare child [%d]", pid);
        smf_server_decrement_spare(state);
    }
}

void smf_server_sig_handler(int sig) {
    /**
     * - SIGUSR1 => child got a new client
     */
    switch(sig) {
        case SIGTERM:
        case SIGINT:
            daemon_exit = 1;
            break;
        case SIGCHLD:
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

    if (sigaction(SIGCHLD, &action, &old_action) < 0) {
        TRACE(TRACE_ERR,"sigaction (SIGCHLD) failed: %s",strerror(errno));
        exit(EXIT_FAILURE);
    }
}

void smf_server_init(SMFSettings_T *settings, SMFServerState_T *state) {
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

        if (chdir(settings->queue_dir) == -1) {
            TRACE(TRACE_ERR, "can't change to queue dir %s",settings->queue_dir, strerror(errno));
            exit(EXIT_FAILURE);
        }
        umask(0);
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

    if (_smf_server_init_ipc(settings,state) < 0) {
        TRACE(TRACE_ERR, "failed to initialize semaphore");
        exit(EXIT_FAILURE);
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

    if (asprintf(&srvname,"%d",settings->bind_port) == -1) {
        TRACE(TRACE_ERR, "failed to set server name");
        return -1;
    }

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

void smf_server_fork(SMFSettings_T *settings, SMFServerState_T *state,
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state)) {
    pid_t pid;

    switch(pid = fork()) {
        case -1:
            TRACE(TRACE_ERR,"fork() failed: %s",strerror(errno));
            break;
        case 0:
            smf_server_accept_handler(settings,state,handle_client_func);
            
            exit(EXIT_SUCCESS); /* quit child process */
            break;
        default: /* parent process: go on with accept */
            TRACE(TRACE_DEBUG,"forked child [%d]",pid);
            break;
    }

    _smf_server_add_child(state,pid);
}

void smf_server_loop(SMFSettings_T *settings, SMFServerState_T *state,
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state)) {
    int i, status;
    pid_t pid;
    SMFServerCounters_T *counters;

    TRACE(TRACE_NOTICE, "starting spmfilter daemon");
    TRACE(TRACE_NOTICE,"binding to %s:%d",settings->bind_ip,settings->bind_port);

    /* prefork min. 1 child(s) */
    if(settings->spare_childs == 0) {
        smf_server_fork(settings,state,handle_client_func);
    } else {
        for (i = 0; i < settings->spare_childs; i++) {
            smf_server_fork(settings,state,handle_client_func);
        }
    }

    for (;;) {
        pid = waitpid(-1, &status, 0);

        if (daemon_exit == 1)
            break;
        if (pid > 0) {
            _smf_server_decrement_proc(state);
            _smf_server_remove_active(state,pid);
        }

        counters = _smf_server_get_counters(state->shm_id); 

        while(counters->num_procs < settings->max_childs) {

            if ((counters->num_spare < settings->spare_childs) || (counters->num_procs < counters->num_spare)) {
                smf_server_fork(settings,state,handle_client_func);
            } else
              break;
        }      

        _smf_server_detach_counters(counters);
    }

    TRACE(TRACE_NOTICE, "stopping spmfilter daemon");
    close(state->sd);

    counters = _smf_server_get_counters(state->shm_id); 
    for (i = 0; i < settings->max_childs; i++)
        if (counters->childs[i] > 0) {
            kill(counters->childs[i],SIGTERM);
        }
    while(wait(NULL) > 0)
        ;

    _smf_server_detach_counters(counters);

    if (semctl(state->sem_id, 0, IPC_RMID) < 0) {
        TRACE(TRACE_ERR,"failed to remove semaphore id %d: %s",state->sem_id,strerror(errno));
    }

    shmctl(state->shm_id, IPC_RMID, NULL);
    free(state->q);
    free(state);

    unlink(settings->pid_file);
}

void smf_server_accept_handler(SMFSettings_T *settings, SMFServerState_T *state, 
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state)) {
    int client;
    socklen_t slen;
    struct sockaddr_storage sa;

    /* process incoming connection in an infinite loop */
    for (;;) {
        slen = sizeof(sa);

        /* accept new connection */
        if ((client = accept(state->sd, (struct sockaddr *)&sa, &slen)) < 0) {
            if (daemon_exit)
                break;

            if (errno != EINTR) {
                TRACE(TRACE_ERR,"accept failed: %s",strerror(errno));
            }
            continue;
        }
        handle_client_func(settings,client,state);
        close(client);
    }

}

