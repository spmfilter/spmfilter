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
#define BUFF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>
#include <cmime.h>
#include <errno.h>

#include "spmfilter_config.h"
#include "smf_core.h"
#include "smf_modules.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_session.h"
#include "smf_lookup.h"
#include "smf_message_private.h"
#include "smf_internal.h"
#include "smf_smtp.h"

#define THIS_MODULE "pipe"
#define BUF_SIZE 1024

static int smf_pipe_handle_q_error(SMFSettings_T *settings, SMFSession_T *session) {
    switch (settings->module_fail) {
        case 1:
            return(1);
        default:
            return(0);
    }
}

static int smf_pipe_handle_q_processing_error(SMFSettings_T *settings, SMFSession_T *session, int retval) {
    if (retval == -1) {
        switch (settings->module_fail) {
            case 1: 
                return(1);
            default:
                return(0);
        }
    } else if(retval == 1) {
        return(1);
    } else if(retval == 2) {
        return(2);
    } else {
        if (session->response_msg != NULL) 
            printf("%s\n",session->response_msg);
        return(1);
    }
    /* if none of the above matched, halt processing, this is just
     * for safety purposes
     */
    TRACE(TRACE_DEBUG, "no conditional matched, will stop queue processing!");
    return(0);
}


/* handle nexthop delivery error */
static int smf_pipe_handle_nexthop_error(SMFSettings_T *settings, SMFSession_T *session) {
    return(0);
}

int load(SMFSettings_T *settings) {
    struct tms start_acct;
    char buffer[BUF_SIZE];
    FILE *spool_file;
    SMFMessage_T *message = smf_message_new();
    SMFSession_T *session = smf_session_new();
    SMFProcessQueue_T *q;
    int ret = -1;

    start_acct = smf_internal_init_runtime_stats();

    /* initialize the modules queue handler */
    q = smf_modules_pqueue_init(
        smf_pipe_handle_q_error,
        smf_pipe_handle_q_processing_error,
        smf_pipe_handle_nexthop_error
    );

    if(q == NULL) {
        return(-1);
    }

    
    /* generate the queue file */
    smf_core_gen_queue_file(settings->queue_dir, &session->message_file, session->id);
    STRACE(TRACE_DEBUG,session->id,"using spool file: '%s'", session->message_file);

    /* open the spool file */
    spool_file = fopen(session->message_file, "w");
    if(spool_file == NULL) {
        STRACE(TRACE_ERR,session->id,"unable to open spool file: %s (%d)",strerror(errno), errno);
        return(-1);
    }

    /* write stream directly to spool_file */
    while(!feof(stdin)) {
        size_t nread, nwritten;

        nread = fread(&buffer, 1, BUF_SIZE-1, stdin);
        if (nread == 0 && ferror(stdin)) {
          STRACE(TRACE_ERR, session->id, "Failed to read from stdin: %s", strerror(errno));
          fclose(spool_file);
          return -1;
        }
        
        nwritten = fwrite(buffer, 1, nread, spool_file);
        if (nread != nwritten) {
          STRACE(TRACE_ERR, session->id, "Failed to write the spoolfile: %s", strerror(errno));
          fclose(spool_file);
          return -1;
        }
    }

    fclose(spool_file);
    if(smf_message_from_file(&message,session->message_file,1) != 0) {
        STRACE(TRACE_ERR, session->id, "smf_message_from_file() failed");
        return(-1);
    }

    session->envelope->message = message;


    ret = smf_modules_process(q,session,settings);
    remove(session->message_file);
    
    TRACE(TRACE_DEBUG,"removing spool file %s",session->message_file);
    
    free(q);
    smf_internal_print_runtime_stats(start_acct,session->id);

    smf_session_free(session);
    return ret;
}

