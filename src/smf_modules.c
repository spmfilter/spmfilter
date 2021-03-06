/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2020 Sebastian Jaekel, Axel Steiner and SpaceNet AG
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
#include <dirent.h>

#include "smf_modules.h"
#include "smf_header.h"
#include "smf_envelope.h"
#include "smf_message.h"
#include "smf_nexthop.h"
#include "smf_trace.h"
#include "smf_internal.h"
#include "smf_dict.h"
#include "smf_smtp.h"

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

SMFModule_T *smf_module_create(SMFSettings_T *settings, const char *name) {
    return smf_module_create_callback(settings, name, NULL);
}

static void *smf_module_create_handle(SMFSettings_T *settings, const char *name) {
    struct stat fstat;
    void *handle;
    char *path;
    char *lib_path;

    if (settings->lib_dir != NULL)
        lib_path = strdup(settings->lib_dir);
    else
        lib_path = strdup(LIB_DIR);

    if (stat(name, &fstat) == 0 && S_ISREG(fstat.st_mode)) {
        path = strdup(name);
    } else if ((path = smf_internal_build_module_path(lib_path, name)) == NULL) {
        TRACE(TRACE_ERR, "failed to build module path for [%s]", name);
        free(lib_path);
        return NULL;
    }
    
    if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
        TRACE(TRACE_ERR, "failed to load module [%s]: %s", name, dlerror());
        free(path);
        free(lib_path);
        return NULL;
    }
        
    free(path);
    free(lib_path);

    return handle;
}

SMFModule_T *smf_module_create_callback(SMFSettings_T *settings, const char *name, ModuleLoadFunction callback) {
    SMFModule_T *module;

    assert(name);

    if ((module = malloc(sizeof(SMFModule_T))) == NULL) {
        return NULL;
    }
    
    module->name = strdup(name);

    if (callback == NULL) {
        module->type = 0;
        module->u.handle = smf_module_create_handle(settings, name);
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
    SMFMessage_T *msg = NULL;
    SMFList_T *initial_headers = NULL;
    SMFListElem_T *elem = NULL;
    SMFModule_T *curmod;
    int ret = 0;
    int mod_count;
    char *header = NULL;
    NexthopFunction nexthop;

    if (smf_list_new(&initial_headers,_header_destroy) != 0) {
        STRACE(TRACE_ERR,session->id, "failed to create header list");
        return -1;
    }

    msg = smf_envelope_get_message(session->envelope);
    elem = smf_list_head(msg->headers);
    while(elem != NULL) {
        smf_list_append(initial_headers,_copy_header(elem));
        elem = elem->next;
    }

    if (settings->add_header == 1)
        if (asprintf(&header,"X-Spmfilter: ") == -1)
            return -1;

    /* fetch user data */
    if (smf_internal_fetch_user_data(settings,session) != 0)
        STRACE(TRACE_ERR, session->id, "failed to load local user data"); 

    mod_count = 0;
    elem = smf_list_head(settings->modules);
    while(elem != NULL) {
        curmod = (SMFModule_T *)smf_list_data(elem);
        elem = elem->next;

        STRACE(TRACE_DEBUG,session->id,"invoke module [%s]", curmod->name);
        ret = smf_module_invoke(settings, curmod, session);
        
        if(ret != 0) {
            ret = q->processing_error(settings,session,ret);
            
            if(ret == 0) {
                STRACE(TRACE_ERR, session->id, "module [%s] failed, stopping processing!", curmod->name);
                free(header);
                smf_list_free(initial_headers);
                return -1;
            } else if(ret == 1) {
                STRACE(TRACE_WARNING, session->id, "module [%s] stopped processing!", curmod->name);
                free(header);
                smf_list_free(initial_headers);
                return 1;
            } else if(ret == 2) {
                STRACE(TRACE_DEBUG,session->id,"module [%s] stopped processing, turning to nexthop processing!",curmod->name);
                break;
            }
        } else {
            STRACE(TRACE_DEBUG, session->id, "module [%s] finished successfully", curmod->name);
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
    char *value1 = NULL;
    char *value2 = NULL;

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
                            value1 = smf_header_get_value(h_msg,i);
                            value2 = smf_header_get_value(h_init,i);
                            if ((value1 != NULL) && (value2 !=NULL) && (strcmp(value1,value2) != 0)) {
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
