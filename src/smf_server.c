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
#include <event2/listener.h>

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

/*
void setnonblock(int fd) {
    int flags;

    flags = fcntl(fd, F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(fd, F_SETFL, flags);
}
*/

void smf_server_sig_handler(int sig) {
    /**
     * - SIGUSR1 => child got a new client
     * - SIGUSR2 => child client closes connections
     */
    switch(sig) {
        case SIGTERM:
        case SIGINT:
            daemon_exit = 1;
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

static void closeAndFreeClient(SMFServerClient_T *client) {
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

static void server_job_function(SMFServerJob_T *job) {
    SMFServerClient_T *client = (SMFServerClient_T *)job->user_data;

    event_base_dispatch(client->evbase);
    closeAndFreeClient(client);
    free(job);
}

/**
 * Kill the server.  This function can be called from another thread to kill
 * the server, causing runServer() to return.
 */
void killServer(void) {
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
    killServer();
}

/**
 * This function will be called by libevent when there is a connection
 * ready to be accepted.
 */
void on_accept(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *arg) {
    SMFServerCallbackArgs_T *callback_args = (SMFServerCallbackArgs_T *)arg;
    SMFServerWorkqueue_T *workqueue = (SMFServerWorkqueue_T *)callback_args->workqueue;
    SMFServerClient_T *client;
    SMFServerJob_T *job;

    /* We got a new connection! Set up a bufferevent for it. */
    //struct event_base *base = evconnlistener_get_base(listener);
    //struct bufferevent *bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    
    /* Create a client object. */
    if ((client = malloc(sizeof(*client))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for client state");
        return;
    }
    memset(client, 0, sizeof(*client));
    client->fd = fd;
    /* Add any custom code anywhere from here to the end of this function
     * to initialize your application-specific attributes in the client struct.
     */

    if ((client->output_buffer = evbuffer_new()) == NULL) {
        TRACE(TRACE_WARNING,"client output buffer allocation failed");
        closeAndFreeClient(client);
        return;
    }

    if ((client->evbase = event_base_new()) == NULL) {
        TRACE(TRACE_WARNING,"client event_base creation failed");
        closeAndFreeClient(client);
        return;
    }

    client->buf_ev = bufferevent_socket_new(client->evbase, fd,
                                            BEV_OPT_CLOSE_ON_FREE);
    if ((client->buf_ev) == NULL) {
        TRACE(TRACE_WARNING,"client bufferevent creation failed");
        closeAndFreeClient(client);
        return;
    }

    callback_args->client = client;



    bufferevent_setcb(client->buf_ev, callback_args->read_cb, callback_args->write_cb, callback_args->event_cb, callback_args);

    bufferevent_enable(client->buf_ev, EV_READ|EV_WRITE);

    /* Create a job object and add it to the work queue. */
    if ((job = malloc(sizeof(*job))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for job state");
        closeAndFreeClient(client);
        return;
    }
    job->job_function = server_job_function;
    job->user_data = client;

    smf_server_workqueue_add_job(workqueue, job);
    TRACE(TRACE_DEBUG,"added job to workqueue");
#if 0
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    SMFServerCallbackArgs_T *callback_args = (SMFServerCallbackArgs_T *)arg;
    SMFServerWorkqueue_T *workqueue = (SMFServerWorkqueue_T *)callback_args->workqueue;
    SMFServerClient_T *client;
    SMFServerJob_T *job;

    client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        TRACE(TRACE_WARNING,"accept failed");
        return;
    }

    /* Set the client socket to non-blocking mode. */
    if (evutil_make_socket_nonblocking(client_fd) < 0) {
        TRACE(TRACE_WARNING,"failed to set client socket to non-blocking");
        close(client_fd);
        return;
    }

    /* Create a client object. */
    if ((client = malloc(sizeof(*client))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for client state");
        close(client_fd);
        return;
    }
    memset(client, 0, sizeof(*client));
    client->fd = client_fd;
    callback_args->client = client;

    /* Add any custom code anywhere from here to the end of this function
     * to initialize your application-specific attributes in the client struct.
     */

    if ((client->output_buffer = evbuffer_new()) == NULL) {
        TRACE(TRACE_WARNING,"client output buffer allocation failed");
        closeAndFreeClient(client);
        return;
    }

    if ((client->evbase = event_base_new()) == NULL) {
        TRACE(TRACE_WARNING,"client event_base creation failed");
        closeAndFreeClient(client);
        return;
    }

    /* Create the buffered event.
     *
     * The first argument is the file descriptor that will trigger
     * the events, in this case the clients socket.
     *
     * The second argument is the callback that will be called
     * when data has been read from the socket and is available to
     * the application.
     *
     * The third argument is a callback to a function that will be
     * called when the write buffer has reached a low watermark.
     * That usually means that when the write buffer is 0 length,
     * this callback will be called.  It must be defined, but you
     * don't actually have to do anything in this callback.
     *
     * The fourth argument is a callback that will be called when
     * there is a socket error.  This is where you will detect
     * that the client disconnected or other socket errors.
     *
     * The fifth and final argument is to store an argument in
     * that will be passed to the callbacks.  We store the client
     * object here.
     */
    client->buf_ev = bufferevent_socket_new(client->evbase, client_fd,
                                            BEV_OPT_CLOSE_ON_FREE);
    if ((client->buf_ev) == NULL) {
        TRACE(TRACE_WARNING,"client bufferevent creation failed");
        closeAndFreeClient(client);
        return;
    }
    bufferevent_setcb(client->buf_ev, smf_smtpd_handle_client, buffered_on_write,
                      buffered_on_error, callback_args);

    /* We have to enable it before our callbacks will be
     * called. */
    bufferevent_enable(client->buf_ev, EV_READ);

    /* Create a job object and add it to the work queue. */
    if ((job = malloc(sizeof(*job))) == NULL) {
        TRACE(TRACE_WARNING,"failed to allocate memory for job state");
        closeAndFreeClient(client);
        return;
    }
    job->job_function = server_job_function;
    job->user_data = client;

    smf_server_workqueue_add_job(workqueue, job);
    TRACE(TRACE_DEBUG,"added job to workqueue");
#endif
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
        struct event_base *base = evconnlistener_get_base(listener);
        int err = EVUTIL_SOCKET_ERROR();
        fprintf(stderr, "Got an error %d (%s) on the listener. "
                "Shutting down.\n", err, evutil_socket_error_to_string(err));

        event_base_loopexit(base, NULL);
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
    listener = evconnlistener_new_bind(evbase_accept, on_accept, (void *) cb,
            LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1,
            (struct sockaddr*)&listen_addr, sizeof(listen_addr));
    if (!listener) {
        TRACE(TRACE_ERR,"Couldn't create listener");
        smf_server_workqueue_shutdown(&workqueue);
        return -1;
    }
    evconnlistener_set_error_cb(listener, accept_error_cb);


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
#if 0
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
            TRACE(TRACE_DEBUG,"forked child [%d]",child[pos]);
            break;
    }
    num_procs++;
}


void smf_server_loop(SMFSettings_T *settings,int sd, SMFProcessQueue_T *q,
        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q)) {
    int i, status;
    pid_t pid;

    TRACE(TRACE_NOTICE, "starting spmfilter daemon");
    TRACE(TRACE_NOTICE,"binding to %s:%d",settings->bind_ip,settings->bind_port);

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

        if (daemon_exit == 1)
            break;
        
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
    }

    TRACE(TRACE_NOTICE, "stopping spmfilter daemon");
	
    close(sd);

    for (i = 0; i < settings->max_childs; i++)
        if (child[i] > 0)
            kill(child[i],SIGTERM);
    while(wait(NULL) > 0)
        ;

    unlink(settings->pid_file);
}
#endif 


//void smf_server_accept_handler(SMFSettings_T *settings, int sd, SMFProcessQueue_T *q, 
//        void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q)) {
void smf_server_accept_handler(int fd, short ev, void *arg) {
    //int client_fd;
    //struct sockaddr_in client_addr;
    //socklen_t client_len = sizeof(client_addr);
    //struct client *client;
   // SMFServerAcceptArgs_T *args = (SMFServerAcceptArgs_T *)arg;
    //SMFServerBufArgs_T *client_args;

    //client_fd = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    //if (client_fd > 0) {
    //    TRACE(TRACE_ERR,"accept failed: %s", strerror(errno));
    //    return;
   // }

    //setnonblock(client_fd);
    
    //client = calloc(1, sizeof( *client));
    //client->fd = client_fd;

//    client_args = (SMFServerBufArgs_T *)calloc(1, sizeof(SMFServerBufArgs_T));
//    client_args->settings = args->settings;
//    client_args->client = client;
//    client->buf_ev = bufferevent_new(client_fd,buf_read_callback,buf_write_callback,buf_error_callback,client_args);
    

//    bufferevent_enable(client->buf_ev, EV_READ);

#if 0
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
#endif
}

