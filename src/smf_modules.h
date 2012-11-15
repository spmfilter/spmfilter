/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Sebastian Jaekel, Axel Steiner and SpaceNet AG
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

#ifndef _SMF_MODULES_H
#define _SMF_MODULES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "smf_session.h"
#include "smf_list.h"

typedef int (*ModuleLoadFunction)(SMFSession_T *session);
typedef int (*LoadEngine)(SMFSettings_T *settings);


/*!
 * @struct SMFModule_T
 * @brief Defines a module
 */
typedef struct {
    uint8_t type; /**< Type of module. If set to 0 then a shared-object is
                       loaded, any other value means that a callback-function
                       is executed. */
    char *name; /**< name of the module */
    union {
        void *handle; /**< module handle, value for typp 0 */
        ModuleLoadFunction callback; /**< Callback, used for type != 0 */
    } u;
} SMFModule_T;

typedef struct {
    int (*nexthop)(SMFSettings_T *settings, SMFSession_T *session);
    int (*load_error)(SMFSettings_T *settings, SMFSession_T *session);
    int (*processing_error)(SMFSettings_T *settings, SMFSession_T *session, int retval);
    int (*nexthop_error)(SMFSettings_T *settings, SMFSession_T *session);
} SMFProcessQueue_T;


/** initialize the process queue */
SMFProcessQueue_T *smf_modules_pqueue_init(
    int (*nexthop)(SMFSettings_T *settings, SMFSession_T *session),
    int (*loaderr)(SMFSettings_T *settings, SMFSession_T *session),
    int (*processerr)(SMFSettings_T *settings, SMFSession_T *session, int retval),
    int (*nhoperr)(SMFSettings_T *settings, SMFSession_T *session));

/**
 * @brief Creates a new module.
 *
 * The module is a shared-object located in the library-path configured during the
 * build of the smpfilter.
 *
 * @param name The name of the module. This is also the name of the library.
 * @return The module-instance. If the shared-library could not be loaded,
 *         NULL is returned.
 */
SMFModule_T *smf_module_create(const char *name);

/**
 * @brief Creates a new new module, which invokes the given callback.
 *
 * Instead of loading an external shared-object, a invocatio of smf_module_invoke()
 * will invoke the given callback.
 *
 * @param name The name of the module.
 * @param callback The callback which is invoked. If you pass NULL here, then
 *                 it still tries to load the shared-object (as fallback).
 * @return The module-instance. If the callback is NULL and the shared-object could
 *         not be located, NULL is returned.
 * @see smf_module_create
 */
SMFModule_T *smf_module_create_callback(const char *name, ModuleLoadFunction callback);

/**
 * @brief Destroys the module-instance again.
 *
 * All the memory allocated for the moduke is released again, unloads the
 * module's shared-library.
 *
 * @param module The module be be destroy
 * @return On success 0 is returned
 */
int smf_module_destroy(SMFModule_T *module);

/**
 * @brief Invokes the module.
 *
 * The functions locates the <code>load</code>-symbol from the module's
 * shared-object. The load-function must be declared like ModuleLoadFunction
 * and should return 0 on success.
 *
 * @param module The module is invoke
 * @param session The session os passed to the load-function of the module
 * @return If the load-function could be located, then the return-code of
 *         the load-invocation is returned. Otherwise -1 is returned.
 */
int smf_module_invoke(SMFModule_T *module, SMFSession_T *session);

/** load all modules and run them */
int smf_modules_process(SMFProcessQueue_T *q, SMFSession_T *session, SMFSettings_T *settings);

/** deliver a message to the nexthop */
int smf_modules_deliver_nexthop(SMFSettings_T *settings, SMFProcessQueue_T *q, SMFSession_T *session);

/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSettings_T *settings, SMFSession_T *session, SMFList_T *initial_headers);

int smf_modules_engine_load(SMFSettings_T *settings);

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_MODULES_H */
