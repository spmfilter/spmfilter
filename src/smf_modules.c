/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Sebastian Jaekel, Axel Steiner and SpaceNet AG
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


#include "smf_modules.h"
#include "smf_internal.h"

#define THIS_MODULE "smf_modules"

/* initialize the processing queue */
ProcessQueue_T *smf_modules_pqueue_init(int(*loaderr)(int module_fail),
        int (*processerr)(int retval, int module_fail, char *response_msg),
        int (*nhoperr)(void *args))
{
    ProcessQueue_T *q;

    q = (ProcessQueue_T *)calloc(1, sizeof(ProcessQueue_T));
    if(q == NULL) {
        TRACE(TRACE_ERR, "failed to allocate memory for process queue!");
        return(NULL);
    }

    q->load_error = loaderr;
    q->processing_error = processerr;
    q->nexthop_error = nhoperr;
    return q;
}

/* build full filename to modules states dir */
static char *smf_modules_stf_path(SMFSettings_T *settings, SMFSession_T *session) {
    char *hex = NULL;
    char buf[1024];

    /* build path to file*/
    // TODO: refactoring for new datatypes
    //hex = smf_core_md5sum(smf_session_header_get(session,"message-id"));
    snprintf(buf, sizeof(buf), "%s/%s.modules", settings->queue_dir, hex);
    free(hex);

    return(strdup(buf));
}

/* write an entry to the state file */
static int smf_modules_stf_write_entry(FILE *fh, char *mod) {
    fprintf(fh, "%s:ok\n", mod);
    return(0);
}

/* load the list of processed modules from file */
static GHashTable *smf_modules_stf_processed_modules(FILE *fh) {
    GHashTable *t;
    char buf[128];
    gchar **parts;

    t = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
    fseek(fh, 0, SEEK_SET); /* rewind the file */

    while(fgets(buf, 128, fh) != NULL) {
        parts = g_strsplit(g_strchomp(buf), ":",2);
        
        if(parts[0] != NULL) {
            g_hash_table_insert(t,
                (gpointer *)(g_strdup(parts[0])),
                (gpointer *)(g_strdup(parts[1]))
            );

            g_strfreev(parts);
        }
    }

    return(t);
}

/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSession_T *session) {
// TODO: reimplement smf_modules_flush_dirty
#if 0
    GMimeStream *stream, *stream2, *stream_filter;
    GMimeParser *parser;
    GMimeMessage *msg;
    GMimeFilter *crlf;
    char *new_queue_file;
    FILE *fd, *fd2;

    if (session->dirty_headers == NULL)
        return 0;

    TRACE(TRACE_DEBUG,"flushing header information to filesystem");

    if ((fd = fopen(session->queue_file,"r")) == NULL) {
        TRACE(TRACE_ERR,"unable to open queue file %s: [%d - %s]\n",
                session->queue_file,errno, strerror(errno));
        return -1;
    }

    if ((stream = g_mime_stream_file_new(fd)) == NULL) {
        TRACE(TRACE_ERR,"failed to create stream: [%d - %s]\n",
                errno, strerror(errno));
        return -1;
    }
    parser = g_mime_parser_new_with_stream(stream);
    msg = g_mime_parser_construct_message(parser);
    g_object_unref(parser);

    while (session->dirty_headers) {
        SMFHeaderModification_T *mod = (SMFHeaderModification_T *)((GSList *)session->dirty_headers)->data;
        switch(mod->status) {
            case HEADER_REMOVE:
                TRACE(TRACE_DEBUG,"removing header [%s]",mod->name);
                g_mime_object_remove_header(GMIME_OBJECT(msg),mod->name);
                break;
            case HEADER_APPEND:
                TRACE(TRACE_DEBUG,"append header [%s] with value [%s]",mod->name,mod->value);
#ifdef HAVE_GMIME24
                g_mime_object_append_header(GMIME_OBJECT(msg),mod->name,mod->value);
#else
                g_mime_object_add_header(GMIME_OBJECT(msg),mod->name,mod->value);
#endif
                break;
            case HEADER_PREPEND:
                TRACE(TRACE_DEBUG,"prepend header [%s] with value [%s]",mod->name,mod->value);
#ifdef HAVE_GMIME24
                g_mime_object_prepend_header(GMIME_OBJECT(msg),mod->name,mod->value);
#else
                g_mime_object_add_header(GMIME_OBJECT(msg),mod->name,mod->value);
#endif
                break;
            case HEADER_SET:
                TRACE(TRACE_DEBUG,"header set");
                g_mime_object_set_header(GMIME_OBJECT(msg),mod->name,mod->value);
                break;
            default:
                break;
        }
        
        g_free(mod->name);
        g_free(mod->value);
        session->dirty_headers = g_slist_remove((GSList *)session->dirty_headers,((GSList *)session->dirty_headers)->data);
        g_slice_free(SMFHeaderModification_T,mod);
    }

    g_mime_stream_flush(stream);
    smf_core_gen_queue_file(&new_queue_file);

    fd2 = fopen(new_queue_file, "w+");
    if(NULL == fd2) {
        TRACE(TRACE_ERR,"failed writing queue file");
        g_object_unref(msg);
        g_object_unref(parser);
        g_object_unref(stream);
        return -1;
    }

    if ((stream2 = g_mime_stream_file_new(fd2)) == NULL) {
        TRACE(TRACE_ERR,"failed to create stream: [%d - %s]\n",
                errno, strerror(errno));
        return -1;
    }

#ifdef HAVE_GMIME24
    if ((stream_filter = g_mime_stream_filter_new(stream2)) == NULL) {
        TRACE(TRACE_ERR,"failed to create stream: [%d - %s]\n",
                errno, strerror(errno));
        return -1;
    }
    crlf = g_mime_filter_crlf_new(TRUE,FALSE);
#else
    if ((stream_filter = g_mime_stream_filter_new_with_stream(stream2)) == NULL) {
        TRACE(TRACE_ERR,"failed to create stream: [%d - %s]\n",
                errno, strerror(errno));
        return -1;
    }
    crlf = g_mime_filter_crlf_new(GMIME_FILTER_CRLF_ENCODE,GMIME_FILTER_CRLF_MODE_CRLF_ONLY);
#endif
    
    g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter), crlf);
    g_mime_object_write_to_stream(GMIME_OBJECT(msg),stream2);
    g_mime_stream_flush(stream2);

    g_object_unref(msg);
    g_object_unref(stream2);
    g_object_unref(stream);
    g_object_unref(stream_filter);
    g_object_unref(crlf);
    if (g_remove(session->queue_file) != 0) {
        TRACE(TRACE_ERR,"failed to remove queue file");
        return -1;
    }

    if(g_rename(new_queue_file,session->queue_file) != 0) {
        TRACE(TRACE_ERR,"failed to rename queue file");
        return -1;
    }

    g_free(new_queue_file);
    g_slist_free((GSList *)session->dirty_headers);
    session->dirty_headers = NULL;
#endif
    return 0;
}

int smf_modules_process(
        ProcessQueue_T *q, SMFSession_T *session, SMFSettings_T *settings) {
    int i;
    int retval;
    ModuleLoadFunction runner;
    gchar *path;
    char *curmod;
    GModule *mod;
    gpointer *sym;
    GHashTable *modlist;
    char *stf_filename = NULL;
    FILE *stfh = NULL;
    gchar *header = NULL;

    /* initialize message file  and load processed modules */
    stf_filename = smf_modules_stf_path(settings,session);
    stfh = fopen(stf_filename, "a+");
    if(stfh == NULL) {
        TRACE(TRACE_ERR, "failed to open message state file => %s", stf_filename);

        if(stf_filename != NULL)
            free(stf_filename);

        return(-1);
    }
    modlist = smf_modules_stf_processed_modules(stfh);

// TODO: fix module loading
#if 0
    for(i=0;settings->modules[i] != NULL; i++) {
        curmod = settings->modules[i];

        /* check if the module is in our modlist, if yes, the module has
         * already been processed and can be skipped */
        if(g_hash_table_lookup(modlist, (gpointer *)curmod) != NULL) {
            TRACE(TRACE_INFO, "skipping modules => %s", curmod);
            continue;
        }

        path = (gchar *)smf_build_module_path(LIB_DIR, curmod);
        if(path == NULL) {
            TRACE(TRACE_DEBUG, "failed to build module path for %s", curmod);
            return(-1);
        }

        TRACE(TRACE_DEBUG, "preparing to run module %s", curmod);

        mod = g_module_open(path, G_MODULE_BIND_LAZY);
        if (!mod) {
            g_free(path);
            TRACE(TRACE_ERR,"module failed to load : %s", g_module_error());

            if(q->load_error(settings->module_fail) == 0)
                return(-1);
            else
                continue;
        }

        if (settings->daemon == 1)
            g_module_make_resident(mod);
        
        if (!g_module_symbol(mod, "load", (gpointer *)&sym)) {
            TRACE(TRACE_ERR,"symbol load could not be foudn : %s", g_module_error());
            g_free(path);
            g_module_close(mod);

            if(q->load_error(settings->module_fail) == 0)
                return(-1);
            else
                continue;
        }

        /* cast spell and execute */
        runner = (ModuleLoadFunction)sym;
        retval = runner(session);

        /* clean up */
        g_free(path);
        g_module_close(mod);
        
        if(retval != 0) {
            retval = q->processing_error(retval, settings->module_fail, session->response_msg);
            
            if(retval == 0) {
                TRACE(TRACE_ERR, "module %s failed, stopping processing!", curmod);
                g_hash_table_destroy(modlist);
                fclose(stfh);
                free(stf_filename);

                return(-1);
            } else if(retval == 1) {
                TRACE(TRACE_WARNING, "module %s stopped processing!", curmod);
                g_hash_table_destroy(modlist);
                fclose(stfh);
                if(unlink(stf_filename) != 0)
                    TRERR("Failed to unlink state file => %s", stf_filename);
                free(stf_filename);
                return(1);
            } else if(retval == 2) {
                TRACE(
                    TRACE_DEBUG,
                    "module %s stopped processing, turning to nexthop processing!",
                    curmod
                );
                break;
            }
        } else {
            TRACE(TRACE_DEBUG, "module %s finished successfully", curmod);
            smf_modules_stf_write_entry(stfh, settings->modules[i]);
        }
    }
#endif
    /* close file, cleanup modlist and remove state file */
    TRACE(TRACE_DEBUG, "module processing finished successfully.");
    fclose(stfh);
    g_hash_table_destroy(modlist);

    if(unlink(stf_filename) != 0) {
        TRERR("Failed to unlink state file => %s", stf_filename);
    }
    free(stf_filename);

    if (settings->add_header == 1) {
        header = g_strdup_printf("processed %s",g_strjoinv(",",settings->modules));
        // TODO: refactoring
//      smf_session_header_append(session,"X-Spmfilter",header);
    }

    g_free(header);
    if (smf_modules_flush_dirty(session) != 0)
        TRACE(TRACE_ERR,"message flush failed");

    /* queue is done, if we're still here check for next hop and
     * deliver
     */
    if (settings->nexthop != NULL ) {
        TRACE(TRACE_DEBUG, "will now deliver to nexthop %s", settings->nexthop);
        return(smf_modules_deliver_nexthop(q, session));
    }
    return(0);
}

int smf_modules_deliver_nexthop(ProcessQueue_T *q,SMFSession_T *session) {
//  int i;
//  SMFMessageEnvelope_T *envelope;
#if 0
    SMFSettings_T *settings = smf_settings_get();

    envelope = smf_message_envelope_new();
    if (session->envelope_from != NULL)
        envelope->from = g_strdup(session->envelope_from->addr);
    else if (session->message_from != NULL)
        envelope->from = g_strdup(session->message_from->addr);
    else
        envelope->from = g_strdup("<>");

    if (session->envelope_to != NULL) {
        /* copy recipients in place */
        for (i = 0; i < session->envelope_to_num; i++) {
            envelope = smf_message_envelope_add_rcpt(envelope,session->envelope_to[i]->addr);
        }
    } else if (session->message_to != NULL) {
        /* copy recipients in place */
        for (i = 0; i < session->message_to_num; i++) {
            envelope = smf_message_envelope_add_rcpt(envelope,session->message_to[i]->addr);
        }

    } else {
        /* hmm....i've no recipients */
        TRACE(TRACE_ERR,"got no recipients");
        return -1;
    }

    envelope->message_file = g_strdup(session->queue_file);
    envelope->nexthop = g_strdup(settings->nexthop);

    /* now deliver, if delivery fails, call error hook */
    // TODO: check if corret nexthop is used!
    if (smf_message_deliver(session->envelope) != 0) {
        TRACE(TRACE_ERR,"delivery to %s failed!",settings->nexthop);
        q->nexthop_error(session);
        return(-1);
    }
#endif
//  smf_message_envelope_unref(envelope);
    return(0);
}

int smf_modules_engine_load(SMFSettings_T *settings, int fd) {
    GModule *module;
    LoadEngine load_engine;
    gchar *engine_path;
    int ret;

    /* check if engine module starts with lib */
    if (settings->lib_dir != NULL)
        engine_path = _build_module_path(settings->lib_dir, settings->engine);
    else
        engine_path = _build_module_path(LIB_DIR, settings->engine);

    /* try to open engine module */
    module = g_module_open(engine_path, G_MODULE_BIND_LAZY);
    if (!module) {
        TRACE(TRACE_ERR,"%s\n", g_module_error());
        return -1;
    }
    
    /* make engine resident, if daemon mode */
    if (settings->daemon == 1)
        g_module_make_resident(module);
    
    /* check if the module provides the function load() */
    if (!g_module_symbol(module, "load", (gpointer *)&load_engine)) {
        TRACE(TRACE_ERR,"%s", g_module_error());
        return -1;
    }

    /* start processing engine */
//    ret = load_engine(settings,fd);

    if (!g_module_close(module))
        TRACE(TRACE_WARNING,"%s", g_module_error());
    g_free(engine_path);

    return ret;
}
