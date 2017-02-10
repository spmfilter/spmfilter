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

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>


#include "smf_settings.h"
#include "smf_trace.h"
#include "smf_server.h"
#include "smf_modules.h"
#include "smf_settings_private.h"


#define THIS_MODULE "server"

// TODO: make configureable
#define NUM_THREADS 2

static struct event_base *evbase_accept;

static SMFServerWorkqueue_T workqueue;
/* Signal handler function (defined below). */
static void sighandler(int signal);


#define LL_ADD(item, list) { \
    item->prev = NULL; \
    item->next = list; \
    list = item; \
}

#define LL_REMOVE(item, list) { \
    if (item->prev != NULL) item->prev->next = item->next; \
    if (item->next != NULL) item->next->prev = item->prev; \
    if (list == item) list = item->next; \
    item->prev = item->next = NULL; \
}

static void *smf_server_worker_function(void *ptr) {
    SMFServerWorker_T *worker = (SMFServerWorker_T *)ptr;
    SMFServerJob_T *job;

    while (1) {
        /* Wait until we get notified. */
        pthread_mutex_lock(&worker->workqueue->jobs_mutex);
        while (worker->workqueue->waiting_jobs == NULL) {
            pthread_cond_wait(&worker->workqueue->jobs_cond,
                              &worker->workqueue->jobs_mutex);
        }
        job = worker->workqueue->waiting_jobs;
        if (job != NULL) {
            LL_REMOVE(job, worker->workqueue->waiting_jobs);
        }
        pthread_mutex_unlock(&worker->workqueue->jobs_mutex);

        /* If we're supposed to terminate, break out of our continuous loop. */
        if (worker->terminate) break;

        /* If we didn't get a job, then there's nothing to do at this time. */
        if (job == NULL) continue;

        /* Execute the job. */
        job->job_function(job);
    }

    free(worker);
    pthread_exit(NULL);
}

void smf_server_close_and_free_client(SMFServerClient_T *client) {
    if (client != NULL) {
        smf_server_close_client(client);
        if (client->buf_ev != NULL) {
            bufferevent_free(client->buf_ev);
            client->buf_ev = NULL;
        }
        if (client->evbase != NULL) {
            event_base_free(client->evbase);
            client->evbase = NULL;
        }
        if (client->output_buffer != NULL) {
            evbuffer_free(client->output_buffer);
            client->output_buffer = NULL;
        }
        free(client);
    }
}

void smf_server_job_function(SMFServerJob_T *job) {
    SMFServerClient_T *client = (SMFServerClient_T *)job->user_data;

    event_base_dispatch(client->evbase);
    smf_server_close_and_free_client(client);
    free(job);
}

static void smf_server_accept_error_cb(struct evconnlistener *listener, void *ctx) {
    struct event_base *base = evconnlistener_get_base(listener);
    int err = EVUTIL_SOCKET_ERROR();
    TRACE(TRACE_ERR, "Got an error %d (%s) on the listener. "
                     "Shutting down.\n", err, evutil_socket_error_to_string(err));

    event_base_loopexit(base, NULL);
}


int smf_server_workqueue_init(SMFServerWorkqueue_T *workqueue, int numWorkers) {
    int i;
    SMFServerWorker_T *worker;
    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;

    if (numWorkers < 1) numWorkers = 1;
    memset(workqueue, 0, sizeof(*workqueue));
    memcpy(&workqueue->jobs_mutex, &blank_mutex,
           sizeof(workqueue->jobs_mutex));
    memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));

    for (i = 0; i < numWorkers; i++) {
        if ((worker = malloc(sizeof(SMFServerWorker_T))) == NULL) {
            perror("Failed to allocate all workers");
            return 1;
        }
        memset(worker, 0, sizeof(*worker));
        worker->workqueue = workqueue;
        if (pthread_create(&worker->thread, NULL, smf_server_worker_function,
                           (void *)worker)) {
            perror("Failed to start all worker threads");
            free(worker);
            return 1;
        }
        LL_ADD(worker, worker->workqueue->workers);
    }

    return 0;
}

void smf_server_workqueue_shutdown(SMFServerWorkqueue_T *workqueue) {
    SMFServerWorker_T *worker = NULL;

    /* Set all workers to terminate. */
    for (worker = workqueue->workers; worker != NULL; worker = worker->next) {
        worker->terminate = 1;
    }

    /* Remove all workers and jobs from the work queue.
     * wake up all workers so that they will terminate. */
    pthread_mutex_lock(&workqueue->jobs_mutex);
    workqueue->workers = NULL;
    workqueue->waiting_jobs = NULL;
    pthread_cond_broadcast(&workqueue->jobs_cond);
    pthread_mutex_unlock(&workqueue->jobs_mutex);
}

void smf_server_workqueue_add_job(SMFServerWorkqueue_T *workqueue, SMFServerJob_T *job) {
    /* Add the job to the job queue, and notify a worker. */
    pthread_mutex_lock(&workqueue->jobs_mutex);
    LL_ADD(job, workqueue->waiting_jobs);
    pthread_cond_signal(&workqueue->jobs_cond);
    pthread_mutex_unlock(&workqueue->jobs_mutex);
}

#if 0
// TODO: replace
void smf_server_sig_handler(int sig) {
    /**
     * - SIGUSR1 => child got a new client
     * - SIGUSR2 => child client closes connections
     */
    switch(sig) {
        case SIGTERM:
        case SIGINT:
            TRACE(TRACE_DEBUG,"DAEMON EXIT [%d] [%d] PID: %d",num_clients, num_spare,getpid());
            break;
        case SIGUSR1:
            num_clients++;
            num_spare--;
            TRACE(TRACE_DEBUG,"NEW CHILD [%d] [%d] PID: %d",num_clients, num_spare,getpid());
            break;
        case SIGUSR2:
            num_clients--;
            TRACE(TRACE_DEBUG,"CLOSE CHILD [%d] [%d] PID: %d",num_clients, num_spare,getpid());
            break;
        default:
            TRACE(TRACE_DEBUG,"DEFAULT");
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
#endif

void smf_server_init(SMFSettings_T *settings) {
    pid_t pid;
    FILE *pidfile;
    
    struct passwd *pwd = NULL;
    struct group *grp = NULL;
   
    // TODO: check sighandler
    //smf_server_sig_init();

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
}

void smf_server_close_client(SMFServerClient_T *client) {
    if (client != NULL) {
        if (client->fd >= 0) {
            close(client->fd);
            client->fd = -1;
        }
    }
}

/**
 * Kill the server.  This function can be called from another thread to kill
 * the server, causing runServer() to return.
 */
void smf_server_kill(void) {
    TRACE(TRACE_INFO, "Stopping socket listener event loop.\n");
    if (event_base_loopexit(evbase_accept, NULL)) {
        perror("Error shutting down server");
    }
    TRACE(TRACE_INFO, "Stopping workers.\n");
    smf_server_workqueue_shutdown(&workqueue);
}


static void sighandler(int signal) {
    TRACE(TRACE_INFO, "Received signal %d: %s.  Shutting down.\n", signal,
            strsignal(signal));
    smf_server_kill();
}

SMFServerClient_T *smf_server_client_create(int fd) {
    SMFServerClient_T *client;

     /* Create a client object. */
    if ((client = malloc(sizeof(*client))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for client state");
        return NULL;
    }
    memset(client, 0, sizeof(*client));
    client->fd = fd;
    /* Add any custom code anywhere from here to the end of this function
     * to initialize your application-specific attributes in the client struct.
     */

    if ((client->output_buffer = evbuffer_new()) == NULL) {
        TRACE(TRACE_WARNING,"client output buffer allocation failed");
        smf_server_close_and_free_client(client);
        return NULL;
    }

    if ((client->evbase = event_base_new()) == NULL) {
        TRACE(TRACE_WARNING,"client event_base creation failed");
        smf_server_close_and_free_client(client);
        return NULL;
    }

    client->buf_ev = bufferevent_socket_new(client->evbase, fd,
                                            BEV_OPT_CLOSE_ON_FREE);
    if ((client->buf_ev) == NULL) {
        TRACE(TRACE_WARNING,"client bufferevent creation failed");
        smf_server_close_and_free_client(client);
        return NULL;
    }

    return client;
}



int smf_server_listen(SMFSettings_T *settings, SMFServerCallbackArgs_T *cb) {
    struct evconnlistener *listener;
    struct sockaddr_in listen_addr;

    /* Set signal handlers */
    // TODO: sighandler in smf_server aufnehmen
    sigset_t sigset;
    sigemptyset(&sigset);
    struct sigaction siginfo = {
        .sa_handler = sighandler,
        .sa_mask = sigset,
        .sa_flags = SA_RESTART,
    };
    sigaction(SIGINT, &siginfo, NULL);
    sigaction(SIGTERM, &siginfo, NULL);

    /* Initialize work queue. */
    if (smf_server_workqueue_init(&workqueue, NUM_THREADS)) {
        TRACE(TRACE_ERR,"Failed to create work queue");
        smf_server_workqueue_shutdown(&workqueue);
        return -1;
    }

    cb->workqueue = &workqueue;

    /* Initialize socket */
    memset(&listen_addr,0,sizeof(listen_addr));
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(settings->bind_port);

    if ((evbase_accept = event_base_new()) == NULL) {
        TRACE(TRACE_ERR,"Unable to create socket accept event base");
        smf_server_workqueue_shutdown(&workqueue);
        return -1;
    }

    // TODO: listener free?
    listener = evconnlistener_new_bind(evbase_accept, cb->accept_cb, (void *) cb,
            LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    if (!listener) {
        TRACE(TRACE_ERR,"Couldn't create listener");
        smf_server_workqueue_shutdown(&workqueue);
        return -1;
    }
    evconnlistener_set_error_cb(listener, smf_server_accept_error_cb);

    smf_server_init(settings);

    TRACE(TRACE_INFO,"Server running.\n");

    /* Start the event loop. */
    event_base_dispatch(evbase_accept);
    event_base_free(evbase_accept);

    TRACE(TRACE_INFO,"Server shutdown.\n");
    return 0;
}

#if 0
int smf_server_listen(SMFSettings_T *settings, SMFServerAcceptArgs_T *accept_args) {
    int fd, reuseaddr;
    int status = -1;
    struct addrinfo hints, *ai, *aptr;
    char *srvname = NULL;
    struct event accept_event;

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
            if ((fd = socket(aptr->ai_family,aptr->ai_socktype, aptr->ai_protocol)) < 0)
                continue;

            reuseaddr = 1;
            //setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int));

            if (bind(fd,aptr->ai_addr,aptr->ai_addrlen) == 0) {
                if (listen(fd, settings->listen_backlog) >= 0)
                    break;
            }
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr));
            setnonblock(fd);

            // TODO: callback richtig setzen
            event_set(&accept_event, fd, EV_READ|EV_PERSIST, smf_server_accept_handler, accept_args);
            event_add(&accept_event,NULL);

            event_dispatch();
            close(fd);
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

    return fd;
}
#endif 
