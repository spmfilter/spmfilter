/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steine, Werner Detter and SpaceNet AG
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
#include "smf_session_private.h"
#include "smf_lookup.h"
#include "smf_message_private.h"
#include "smf_internal.h"

#define THIS_MODULE "pipe"
#define BUF_SIZE 1024

/* error handler used when building module queue
 * return 1 if processing should continue, else 0
 */
static int handle_q_error(int module_fail) {
    switch (module_fail) {
        case 1:
            return(1);
        default:
            return(0);
    }
}

/* handle processing errors when running queue
 */
static int handle_q_processing_error(int retval, int module_fail, char *response_msg) {
    if (retval == -1) {
        switch (module_fail) {
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
        if (response_msg != NULL) 
            printf("%s\n",response_msg);
        return(1);
    }
    /* if none of the above matched, halt processing, this is just
     * for safety purposes
     */
    TRACE(TRACE_DEBUG, "no conditional matched, will stop queue processing!");
    return(0);
}


/* handle nexthop delivery error */
static int handle_nexthop_error(void *args) {
    return(0);
}

int load_modules(SMFSession_T *session, SMFSettings_T *settings) {
    int ret;
    ProcessQueue_T *q;

    /* initialize the modules queue handler */
    q = smf_modules_pqueue_init(
        handle_q_error,
        handle_q_processing_error,
        handle_nexthop_error
    );

    if(q == NULL) {
        return(-1);
    }

    /* now tun the process queue */
    ret = smf_modules_process(q,session,settings);
    free(q);

    if(ret != 0) {
        TRACE(TRACE_DEBUG, "pipe engine failed to process modules!");
        return(-1);
    }
    return(0);
}

int load(SMFSettings_T *settings,int sock) {
    struct tms start_acct;
    char buffer[BUF_SIZE];
    FILE *spool_file;
    struct stat st;
    SMFMessage_T *message = smf_message_new();
    SMFSession_T *session = smf_session_new();

    start_acct = _init_runtime_stats();

    /* generate the queue file */
    smf_core_gen_queue_file(settings->queue_dir, &session->message_file, session->id);
    TRACE(TRACE_DEBUG,"using spool file: '%s'", session->message_file);

    /* open the spool file */
    spool_file = fopen(session->message_file, "w");
    if(spool_file == NULL) {
        TRACE(TRACE_ERR,"unable to open spool file: %s (%d)",strerror(errno), errno);
        return(-1);
    }

    /* write stream directly to spool_file */
    while(!feof(stdin)) {
        char *content_to_write = malloc(sizeof(char) * BUF_SIZE);

        if(content_to_write == NULL) {
            TRACE(TRACE_ERR, "Failed to reallocate memory for content");
            fclose(spool_file);
            return(-1);
        }

        content_to_write[0] = '\0';
        fread(&buffer, BUF_SIZE-1, sizeof(char), stdin);
        strcat(content_to_write, buffer);
        fwrite(content_to_write, sizeof(char), strlen(content_to_write), spool_file);
        free(content_to_write);
    }

    fclose(spool_file);
    if(smf_message_from_file(&message,session->message_file,1) != 0) {
        TRACE(TRACE_ERR, "smf_message_from_file() failed");
        return(-1);
    }

    session->envelope->message = message;

    
    /*
    if (load_modules(session, settings) != 0) {
        remove(session->message_file);
        smf_session_free(session);
        TRACE(TRACE_DEBUG,"removing spool file %s",session->message_file);
        return -1;
    } else {
        remove(session->message_file);
        smf_session_free(session);
        TRACE(TRACE_DEBUG,"removing spool file %s",session->message_file);
        return 0;
    }
    */
    
    _print_runtime_stats(start_acct);

    return 0;
}
