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
#include <event2/event.h>
#include <event2/listener.h>

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
    void *user_data; /**< SMFServerClient_T */
    struct job *prev;
    struct job *next;
} SMFServerJob_T;

typedef struct workqueue {
    struct worker *workers;
    struct job *waiting_jobs;
    pthread_mutex_t jobs_mutex;
    pthread_cond_t jobs_cond;
} SMFServerWorkqueue_T;

int smf_server_workqueue_init(SMFServerWorkqueue_T *workqueue, int numWorkers);
void smf_server_workqueue_shutdown(SMFServerWorkqueue_T *workqueue);
void smf_server_workqueue_add_job(SMFServerWorkqueue_T *workqueue, SMFServerJob_T *job);

/**
 * Struct to carry around connection (client)-specific data.
 */
typedef struct {
    int fd; /**< The client's socket. */
    struct event_base *evbase; /**< The event_base for this client. */
    struct bufferevent *buf_ev; /**< The bufferedevent for this client. */
    char *client_addr;
} SMFServerClient_T;

typedef struct {
  SMFSettings_T *settings;
  SMFServerWorkqueue_T *workqueue;
  SMFProcessQueue_T *q;
  SMFServerClient_T *client;
  SMFSession_T *session;
  void (*accept_cb)(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,void *arg);
  void *engine_data;
} SMFServerEngineCtx_T;

void smf_server_sig_init(void);
void smf_server_sig_handler(int sig);
void smf_server_init(SMFSettings_T *settings);
int smf_server_listen(SMFSettings_T *settings, SMFServerEngineCtx_T *cb);
void smf_server_close_client(SMFServerClient_T *client);
void smf_server_close_and_free_client(SMFServerClient_T *client);
void smf_server_job_function(SMFServerJob_T *job);
SMFServerClient_T *smf_server_client_create(int fd, struct sockaddr *addr, int addrlen);
void smf_server_event_cb(struct bufferevent *bev, short events, void *arg);

#endif  /* _SMF_SERVER_H */

