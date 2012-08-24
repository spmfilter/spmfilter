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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <unistd.h>
#include <fcntl.h>
#include <cmime.h>

#include "spmfilter_config.h"
#include "smf_core.h"
#include "smf_modules.h"
#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_session.h"
#include "smf_session_private.h"
#include "smf_lookup.h"
#include "smf_message_private.h"

#define THIS_MODULE "pipe"


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
            g_printerr("%s\n",response_msg);
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
    GIOChannel *in_channel, *out_channel;
    gchar buf[100];
    gsize bytes_read,bytes_written;
    GError *error = NULL;
    clock_t start_process, stop_process;

    SMFSession_T *session = smf_session_new();

    /* start clock, to see how long
     * the processing time takes */
    start_process = clock();

    smf_core_gen_queue_file(settings->queue_dir, &session->envelope->message_file);

    TRACE(TRACE_DEBUG,"using spool file: '%s'", session->envelope->message_file);
        
    /* start receiving data */
    in_channel = g_io_channel_unix_new(STDIN_FILENO);
    g_io_channel_set_encoding(in_channel, NULL, NULL);

    out_channel = g_io_channel_new_file(session->envelope->message_file,"w",&error);
    g_io_channel_set_encoding(out_channel, NULL, NULL);
    if(!out_channel) {
        TRACE(TRACE_ERR,"failed writing queue file: %s",error->message);
        g_error_free(error);
        return -1;
    }


    do {
        g_io_channel_read_chars(in_channel,buf,100,&bytes_read,&error);
        if(error) {
            TRACE(TRACE_ERR,"error while reading: %s",error->message);
            g_error_free(error);
            return -1;
        }
        
        g_io_channel_write_chars(out_channel,buf,bytes_read,&bytes_written,&error);
        if(error) {
            TRACE(TRACE_ERR,"error while writing queue file: %s",error->message);
            g_error_free(error);
            return -1;
        }
        session->msgbodysize+=bytes_written;
    }
    while(bytes_read > 0);

    g_io_channel_unref(in_channel);
    g_io_channel_shutdown(out_channel,TRUE,&error);
    if(error){
        TRACE(TRACE_ERR,"failed to close queue file: %s",error->message);
        g_error_free(error);
        return 1;
    }

    TRACE(TRACE_DEBUG,"data complete, message size: %d", (u_int32_t)session->msgbodysize);
//    session->envelope->num_rcpts = 0;
    
    
    
    /* parse email data and fill session struct*/
    /* extract message headers */
/*
    g_mime_stream_flush(out);
    g_mime_stream_seek(out,0,0);
    parser = g_mime_parser_new_with_stream(out);
    message = GMIME_OBJECT(g_mime_parser_construct_message(parser));
*/
//    smf_message_extract_addresses(&session->envelope);

#if 0
#ifdef HAVE_GMIME24
    headers = (void *)g_mime_object_get_header_list(message);
    session->envelope->message->headers = (void *)g_mime_header_list_new();
    g_mime_header_list_foreach(headers, copy_header_func, session->envelope->message->headers);
#else
    headers = (void *)g_mime_object_get_headers(message);
    session->envelope->message->headers = (void *)g_mime_header_new();
    g_mime_header_foreach(headers, copy_header_func, session->envelope->message->headers);
#endif
#endif
//    g_object_unref(parser);
//    g_object_unref(message);
//    g_object_unref(out);
//    g_io_channel_unref(in);

    /* processing is done, we can
     * stop our clock */
    stop_process = clock();
    TRACE(TRACE_DEBUG,"processing time: %0.5f sec.", (float)(stop_process-start_process)/CLOCKS_PER_SEC);
/*
    if (load_modules(session, settings) != 0) {
        remove(session->envelope->message_file);
        smf_session_free(session);
        TRACE(TRACE_DEBUG,"removing spool file %s",session->envelope->message_file);
        return -1;
    } else {
        remove(session->envelope->message_file);
        smf_session_free(session);
        TRACE(TRACE_DEBUG,"removing spool file %s",session->envelope->message_file);
        return 0;
    }
  */  
    return 0;
}
