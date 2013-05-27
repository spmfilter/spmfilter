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

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "smf_modules.h"
#include "smf_header.h"
#include "smf_envelope.h"
#include "smf_message.h"
#include "smf_nexthop.h"
#include "smf_trace.h"
#include "smf_internal.h"
#include "smf_dict.h"
#include "smf_smtp.h"
#include "smf_lookup.h"

#define THIS_MODULE "modules"

static time_t message_file_mtime(SMFSession_T *session) {
  struct stat fstat;
  
  if (stat(session->message_file, &fstat) != 0) {
    STRACE(TRACE_ERR, session->id, "%s: %s", session->message_file, strerror(errno));
    return 0; /* Ignore... */
  }
  
  return fstat.st_mtime;
}

void _header_destroy(void *data) {
    SMFHeader_T *h = (SMFHeader_T *)data;
    smf_header_free(h);
}

void *_copy_header(SMFListElem_T *elem) {
    SMFHeader_T *o = (SMFHeader_T *)smf_list_data(elem);
    SMFHeader_T *n = smf_header_new();
    char *s = NULL;
    int i;

    s = smf_header_get_name(o);
    smf_header_set_name(n, s);

    for(i=0; i < smf_header_get_count(o); i++) {
        s = smf_header_get_value(o, i);
        smf_header_set_value(n, s, 0);
    } 

    return n;
}

int smf_modules_engine_load(SMFSettings_T *settings) {
    void *module = NULL;
    LoadEngine load_engine = NULL;
    char *engine_path = NULL;
    char *error = NULL;
    int ret;

    /* check if engine module starts with lib */
    if (settings->lib_dir != NULL)
        engine_path = smf_internal_build_module_path(settings->lib_dir, settings->engine);
    else
        engine_path = smf_internal_build_module_path(LIB_DIR, settings->engine);

    if ((module = dlopen(engine_path, RTLD_LAZY)) == NULL) {
        TRACE(TRACE_ERR,"failed to load library [%s]: %s", engine_path,dlerror());
        free(engine_path);
        return -1;
    }
    dlerror();  

    load_engine = dlsym(module,"load");
    if ((error = dlerror()) != NULL)  {
        TRACE(TRACE_ERR,"library error: %s", error);
        free(error);
        free(engine_path);
        return -1;
    }

    ret = load_engine(settings);

    if (dlclose(module) != 0) {
        TRACE(TRACE_ERR,"failed to unload module [%s]",engine_path);
    }

    free(engine_path);

    return ret;
}

/* initialize the processing queue */
SMFProcessQueue_T *smf_modules_pqueue_init(
        int (*loaderr)(SMFSettings_T *settings, SMFSession_T *session),
        int (*processerr)(SMFSettings_T *settings, SMFSession_T *session, int retval),
        int (*nhoperr)(SMFSettings_T *settings, SMFSession_T *session)) {
    SMFProcessQueue_T *q;

    q = (SMFProcessQueue_T *)calloc(1, sizeof(SMFProcessQueue_T));
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
    char *mid = NULL;
    char *buf = NULL;
    SMFMessage_T *msg = NULL;
    msg = smf_envelope_get_message(session->envelope);

    mid = smf_message_get_message_id(msg);
    hex = smf_core_md5sum(mid);
    asprintf(&buf,"%s/%s.%s.modules", settings->queue_dir, session->id, hex);
    free(hex);

    return(buf);
}

/* load the list of processed modules from file */
static SMFDict_T *smf_modules_stf_processed_modules(FILE *fh) {
    SMFDict_T *d;
    char *buf = NULL;
    size_t n;
    char **parts = NULL;
    int nparts;

    d = smf_dict_new();
    fseek(fh, 0, SEEK_SET); /* rewind the file */

    while(getline(&buf,&n,fh) >= 0) {
        parts = smf_core_strsplit(buf, ":", &nparts);
        assert(nparts == 2);
        smf_dict_set(d, parts[0], parts[1]);
    }

    free(buf);
    if (parts != NULL) {
        free(parts[0]);
        free(parts[1]);
        free(parts[2]);
        free(parts);
    }
    return d;
}

/* write an entry to the state file */
static int smf_modules_stf_write_entry(FILE *fh, char *mod) {
    fprintf(fh, "%s:ok\n", mod);
    return(0);
}

SMFModule_T *smf_module_create(const char *name) {
    return smf_module_create_callback(name, NULL);
}

static void *smf_module_create_handle(const char *name) {
    struct stat fstat;
    void *handle;
    char *path;

    if (stat(name, &fstat) == 0 && S_ISREG(fstat.st_mode)) {
        path = strdup(name);
    } else if ((path = smf_internal_build_module_path(LIB_DIR, name)) == NULL) {
        TRACE(TRACE_ERR, "failed to build module path for [%s]", name);
        return NULL;
    }
    
    if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
        TRACE(TRACE_ERR, "failed to load module [%s]: %s", name, dlerror());
        free(path);
        return NULL;
    }
        
    free(path);
    
    return handle;
}

SMFModule_T *smf_module_create_callback(const char *name, ModuleLoadFunction callback) {
    SMFModule_T *module;

    assert(name);

    if ((module = malloc(sizeof(SMFModule_T))) == NULL) {
        return NULL;
    }
    
    module->name = strdup(name);

    if (callback == NULL) {
        module->type = 0;
        module->u.handle = smf_module_create_handle(name);
    } else {
        module->type = 1;
        module->u.callback = callback;
    }
    
    TRACE(TRACE_DEBUG, "module %s loaded", name);
    
    return module;
}

int smf_module_destroy(SMFModule_T *module) {
    int result = 0;
    
    assert(module);

    if (module->type == 0) {
        if (module->u.handle != NULL && dlclose(module->u.handle) != 0) {
            TRACE(TRACE_ERR, "failed to unload module [%s]", module->name);
            result = -1;
        }
    }

    free(module->name);
    free(module);
    
    return result;
}

int smf_module_invoke(SMFSettings_T *settings, SMFModule_T *module, SMFSession_T *session) {
    ModuleLoadFunction runner;
    time_t mtime_before, mtime_after;
    int result;
    
    assert(module);
    assert(session);
    
    if (module->type == 0) {
        dlerror(); // Clear any errors
        if ((runner = dlsym(module->u.handle, "load")) == NULL) {
            TRACE(TRACE_ERR, "failed to locate 'load'-symbol in module '%s': %s",
                  module->name, dlerror());
              return -1;
          }
    } else {
        runner = module->u.callback;
    }
    
    if (session->message_file != NULL)
      mtime_before = message_file_mtime(session);
    
    result = runner(settings,session);

    if (result == 0 && session->message_file != NULL) {
      mtime_after = message_file_mtime(session);
      
      if (mtime_after > mtime_before) {
        // Spoolfile has change. Reload the message inside the session
        SMFMessage_T *message_new = smf_message_new();
        result = smf_message_from_file(&message_new, session->message_file, 0);
        
        if (result == 0) {
          smf_message_free(session->envelope->message);
          session->envelope->message = message_new;
        }
      }
    }
    
    return result;
}

int smf_modules_process(
        SMFProcessQueue_T *q, SMFSession_T *session, SMFSettings_T *settings) {
    FILE *stfh = NULL;
    char *stf_filename = NULL;
    SMFDict_T *modlist;
    SMFMessage_T *msg = NULL;
    SMFList_T *initial_headers = NULL;
    SMFListElem_T *elem = NULL;
    SMFModule_T *curmod;
    int ret = 0;
    int mod_count;
    char *header = NULL;
    NexthopFunction nexthop;

    /* initialize message file  and load processed modules */
    stf_filename = smf_modules_stf_path(settings,session);

    stfh = fopen(stf_filename, "a+");
    if(stfh == NULL) {
        STRACE(TRACE_ERR, session->id, "failed to open message state file %s: %s (%d)", stf_filename, strerror(errno),errno);

        if(stf_filename != NULL)
            free(stf_filename);

        return -1;
    }

    if (smf_list_new(&initial_headers,_header_destroy) != 0) {
        STRACE(TRACE_ERR,session->id, "failed to create header list");
        free(stf_filename);
        fclose(stfh);
        return -1;
    }

    msg = smf_envelope_get_message(session->envelope);
    elem = smf_list_head(msg->headers);
    while(elem != NULL) {
        smf_list_append(initial_headers,_copy_header(elem));
        elem = elem->next;
    }

    if (settings->add_header == 1)
        asprintf(&header,"X-Spmfilter: ");

    /* fetch user data */
    if (smf_modules_fetch_user_data(settings,session) != 0)
        STRACE(TRACE_ERR, session->id, "failed to load local user data"); 

    mod_count = 0;
    modlist = smf_modules_stf_processed_modules(stfh);
    elem = smf_list_head(settings->modules);
    while(elem != NULL) {
        curmod = (SMFModule_T *)smf_list_data(elem);
        elem = elem->next;

        /* check if the module is in our modlist, if yes, the module has
         * already been processed and can be skipped */
        if(smf_dict_get(modlist, curmod->name) != NULL) {
            STRACE(TRACE_INFO, session->id, "skipping module [%s]", curmod->name);
            continue;
        }

        ret = smf_module_invoke(settings, curmod, session);

        if(ret != 0) {
            ret = q->processing_error(settings,session,ret);
            
            if(ret == 0) {
                STRACE(TRACE_ERR, session->id, "module [%s] failed, stopping processing!", curmod->name);
                smf_dict_free(modlist);
                fclose(stfh);
                free(stf_filename);
                free(header);
                smf_list_free(initial_headers);
                return -1;
            } else if(ret == 1) {
                STRACE(TRACE_WARNING, session->id, "module [%s] stopped processing!", curmod->name);
                smf_dict_free(modlist);
                fclose(stfh);
                if(unlink(stf_filename) != 0)
                    STRACE(TRACE_ERR,session->id,"Failed to unlink state file [%s]", stf_filename);
                free(stf_filename);
                free(header);
                smf_list_free(initial_headers);
                return 1;
            } else if(ret == 2) {
                STRACE(TRACE_DEBUG,session->id,"module [%s] stopped processing, turning to nexthop processing!",curmod->name);
                break;
            }
        } else {
            STRACE(TRACE_DEBUG, session->id, "module [%s] finished successfully", curmod->name);
            smf_modules_stf_write_entry(stfh, curmod->name);
        }

        mod_count++;
        if (settings->add_header == 1) {
            if (mod_count == smf_list_size(settings->modules))
                smf_core_strcat_printf(&header, "%s", curmod->name);
            else
                smf_core_strcat_printf(&header, "%s, ", curmod->name);
        }
    }

    /* close file, cleanup modlist and remove state file */
    STRACE(TRACE_DEBUG, session->id,"module processing finished successfully.");
    fclose(stfh);
    smf_dict_free(modlist);

    if(unlink(stf_filename) != 0) {
        STRACE(TRACE_ERR,session->id,"failed to unlink state file [%s]: %s (%d)", stf_filename,strerror(errno),errno);
    }
    free(stf_filename);
   
    if ((ret == 0) || (ret == 2)) {
        if (settings->add_header == 1) {
            smf_message_set_header(msg, header);
            free(header); 
        }
        
        if ((ret = smf_modules_flush_dirty(settings,session,initial_headers)) != 0)
            STRACE(TRACE_ERR,session->id,"message flush failed");

        /* queue is done, if we're still here check for next hop and
         * deliver
         */
        if (ret == 0 && (nexthop = smf_nexthop_find(settings)) != NULL) {
            if ((ret = nexthop(settings, session)) != 0)
                q->nexthop_error(settings, session);
        }
    }
    smf_list_free(initial_headers);

    return ret;
}


/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSettings_T *settings, SMFSession_T *session, SMFList_T *initial_headers) {
    SMFHeader_T *h_init = NULL;
    SMFHeader_T *h_msg = NULL;
    SMFListElem_T *elem_init = NULL;
    SMFListElem_T *elem_msg = NULL;
    SMFMessage_T *msg = NULL;
    int dirty = 0;
    int found = 0;
    int i = 0;

    STRACE(TRACE_DEBUG,session->id,"flushing header information to filesystem");
    
    msg = smf_envelope_get_message(session->envelope);

    elem_msg = smf_list_head(msg->headers);

    /* check if message headers have changed during session */
    if (msg->headers->size != initial_headers->size) {
        dirty = 1;
    } else {
        while(elem_msg != NULL) {
            h_msg = (SMFHeader_T *)smf_list_data(elem_msg);
            found = 0;
            elem_init = smf_list_head(initial_headers);
            while (elem_init != NULL) {
                h_init = (SMFHeader_T *)smf_list_data(elem_init);
                /* ok, we found the header name in the initial list */
                if (strcmp(h_msg->name,h_init->name)==0) {
                    found = 1;
                    /* lets check if the values are the same */
                    if (smf_header_get_count(h_msg) != smf_header_get_count(h_init)) {
                        dirty = 1;
                    } else {
                        for (i=0;i<smf_header_get_count(h_msg);i++) {
                            if (strcmp(smf_header_get_value(h_msg,i),smf_header_get_value(h_init,i)) != 0) {
                                dirty = 1;
                            }
                        }
                    }   
                    break;
                }
                elem_init = elem_init->next;
            }

            if (found == 0) {
                dirty = 1;
                break;
            }
            elem_msg = elem_msg->next;
        }   
    }
    
    /* merge new headers with message content to new queue file */
    if (dirty == 1) {
        char tmpname[PATH_MAX];
        int fd;
        FILE *new = NULL;
        FILE *old = NULL;
        
        snprintf(tmpname, sizeof(tmpname), "%s/XXXXXX", settings->queue_dir);

        if ((fd = mkstemp(tmpname)) == -1) {
            STRACE(TRACE_ERR,session->id,"failed to create temporary file: %s (%d)",strerror(errno),errno);
            return -1;
        }
    
        if (smf_message_to_fd(msg, fd) == -1) {
            STRACE(TRACE_ERR,session->id,"unable to write temporary file [%s]: %s",tmpname, strerror(errno));
            return -1;
        }
        
        close(fd);

        if((new = fopen(tmpname, "a"))==NULL) {
            STRACE(TRACE_ERR,session->id,"unable to open temporary file: %s (%d)",strerror(errno), errno);
            return -1;
        }

        if((old = fopen(session->message_file, "r"))==NULL) {
            STRACE(TRACE_ERR,session->id,"unable to open queue file: %s (%d)",strerror(errno), errno);
            fclose(new);
            return -1;
        }

        if (smf_message_write_skip_header(old, new) == -1) {
            fclose(old); 
            fclose(new);
            return -1;    
        }
        
        fclose(old); 
        fclose(new);

        if (unlink(session->message_file)!=0) {
            STRACE(TRACE_ERR,session->id,"failed to remove queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }

        if (rename(tmpname,session->message_file)!=0) {
            STRACE(TRACE_ERR,session->id,"failed to rename queue file: %s (%d)",strerror(errno),errno);
            return -1;
        }
    }

    return 0;
}

int smf_modules_deliver_nexthop(SMFSettings_T *settings, SMFProcessQueue_T *q, SMFSession_T *session) {
    SMFEnvelope_T *env = smf_session_get_envelope(session);
    SMFSmtpStatus_T *status = NULL;

    if (env->sender == NULL)
        smf_envelope_set_sender(env, "<>");

    if (env->recipients->size == 0) {
        STRACE(TRACE_ERR,session->id,"got no recipients");
        return -1;
    }

    if (env->nexthop == NULL)
        smf_envelope_set_nexthop(env, settings->nexthop);

    status = smf_smtp_deliver(env, settings->tls, session->message_file,session->id);
    if (status->code != 250) {
        STRACE(TRACE_ERR,session->id,"delivery to [%s] failed!",settings->nexthop);
        STRACE(TRACE_ERR,session->id,"nexthop said: %d - %s", status->code,status->text);
        q->nexthop_error(settings, session);
        return -1;
    }

    smf_smtp_status_free(status);

    return 0;
}

int smf_modules_fetch_user_data(SMFSettings_T *settings, SMFSession_T *session) {
    SMFListElem_T *e = NULL;
    char *addr = NULL;
    char *query = NULL;
    
    if ((strcmp(settings->backend,"ldap")==0) && (settings->ldap_user_query==NULL)) {
        STRACE(TRACE_WARNING, session->id, "no user_query defined for ldap backend");
        return 0; 
    }

    if ((strcmp(settings->backend,"sql")==0) && (settings->sql_user_query==NULL)) {
        STRACE(TRACE_WARNING, session->id, "no user_query defined for sql backend");
        return 0; 
    }

    e = smf_list_head(session->envelope->recipients);
    while (e != NULL) {
        addr = (char *)smf_list_data(e);
        STRACE(TRACE_DEBUG, session->id, "fetching user data for [%s]", addr);

#ifdef HAVE_LDAP
        if (smf_core_expand_string(settings->ldap_user_query,addr,&query) == -1) {
            STRACE(TRACE_ERR, session->id, "failed to expand user query");
            return -1;
        }

        session->local_users = smf_lookup_ldap_query(settings, session, query);
#elif defined HAVE_SQL
        if (smf_core_expand_string(settings->sql_user_query,addr,&query) == -1) {
            STRACE(TRACE_ERR, session->id, "failed to expand user query");
            return -1;
        }

        session->local_users = smf_lookup_sql_query(settings, session, query);
#endif

        if (query != NULL) {
            free(query);
            query = NULL;
        }
        e = e->next;
    }
    return 0;
}
