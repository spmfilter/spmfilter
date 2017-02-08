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

#ifndef _SMF_SERVER_H
#define _SMF_SERVER_H

#include <pthread.h>

#include "smf_settings.h"
#include "smf_modules.h"

typedef struct worker {
    pthread_t thread;
    int terminate;
    struct workqueue *workqueue;
    struct worker *prev;
    struct worker *next;
} SMFServerWorker_T;

typedef struct job {
    void (*job_function)(struct job *job);
    void *user_data;
    struct job *prev;
    struct job *next;
} SMFServerJob_t;

typedef struct workqueue {
    struct worker *workers;
    struct job *waiting_jobs;
    pthread_mutex_t jobs_mutex;
    pthread_cond_t jobs_cond;
} SMFServerWorkqueue_T;

int smf_server_workqueue_init(SMFServerWorkqueue_T *workqueue, int numWorkers);

void smf_server_workqueue_shutdown(SMFServerWorkqueue_T *workqueue);

void smf_server_workqueue_add_job(SMFServerWorkqueue_T *workqueue, job_t *job);

//typedef void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q);

/**
 * Struct to carry around connection (client)-specific data.
 */
typedef struct {
    /* The client's socket. */
    int fd;

    /* The event_base for this client. */
    struct event_base *evbase;

    /* The bufferedevent for this client. */
    struct bufferevent *buf_ev;

    /* The output buffer for this client. */
    struct evbuffer *output_buffer;

    /* Here you can add your own application-specific attributes which
     * are connection-specific. */
} SMFServerClient_T;

typedef struct {
  SMFSettings_T *settings;
  SMFServerWorkqueue_T *workqueue;
  SMFProcessQueue_T *q;
} SMFServerCallbackArgs_T;

void smf_server_sig_init(void);
void smf_server_sig_handler(int sig);
void smf_server_init(SMFSettings_T *settings);
int smf_server_listen(SMFSettings_T *settings, SMFServerAcceptArgs_T *args);


//void smf_server_fork(SMFSettings_T *settings,int sd,SMFProcessQueue_T *q,
//    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));
//void smf_server_loop(SMFSettings_T *settings,int sd,SMFProcessQueue_T *q,
//    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));

//void smf_server_accept_handler(
//    SMFSettings_T *settings, 
//    int sd, 
//    SMFProcessQueue_T *q,
//    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFProcessQueue_T *q));


void smf_server_accept_handler(int fd, short ev, void *arg);

void buf_error_callback(struct bufferevent *bev, short what, void *arg);
//void setnonblock(int fd);
#endif  /* _SMF_SERVER_H */

