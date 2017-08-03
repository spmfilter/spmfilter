/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2017 Axel Steiner and SpaceNet AG
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

#include "smf_settings.h"
#include "smf_modules.h"
#include "spmfilter_config.h"

#ifdef HAVE_SEMAPHORE
#include <semaphore.h>
#define SHMOBJ_PATH "/shmspmfilter"
#define SNAME "/spmfilter"
#endif

typedef struct {
  int num_procs;
  int num_spare;
  int max_childs;
  int childs_active[CHILD_LIMIT];
  int childs[CHILD_LIMIT];
} SMFServerCounters_T;

typedef struct {
#ifdef HAVE_SEMAPHORE
  sem_t *sem_id;
  int shm_fd;
#else
  int sem_id;
  int shm_id;
  key_t sem_key;
  key_t shm_key;
#endif
  int sd;
  SMFProcessQueue_T *q;
  SMFServerCounters_T *counters;
} SMFServerState_T;

typedef void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state);

void smf_server_sig_init(void);
void smf_server_sig_handler(int sig);

void smf_server_init(SMFSettings_T *settings, SMFServerState_T *state);
int smf_server_listen(SMFSettings_T *settings);

void smf_server_fork(SMFSettings_T *settings, SMFServerState_T *state,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state));

void smf_server_loop(SMFSettings_T *settings, SMFServerState_T *state,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state));

void smf_server_accept_handler(
    SMFSettings_T *settings, 
    SMFServerState_T *state,
    void (*handle_client_func)(SMFSettings_T *settings,int client,SMFServerState_T *state));

void smf_server_decrement_spare(SMFServerState_T *state);
void smf_server_add_active(SMFServerState_T *state, int pid);

#endif  /* _SMF_SERVER_H */

