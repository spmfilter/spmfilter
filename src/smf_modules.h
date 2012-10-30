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

#include "smf_session.h"
#include "smf_settings.h"
#include "smf_list.h"

typedef int (*ModuleLoadFunction)(SMFSession_T *session);
typedef int (*LoadEngine)(SMFSettings_T *settings);

typedef struct {
    int (*load_error)(SMFSettings_T *settings, SMFSession_T *session);
    int (*processing_error)(SMFSettings_T *settings, SMFSession_T *session, int retval);
    int (*nexthop_error)(SMFSettings_T *settings, SMFSession_T *session);
} SMFProcessQueue_T;


/** initialize the process queue */
SMFProcessQueue_T *smf_modules_pqueue_init(
    int (*loaderr)(SMFSettings_T *settings, SMFSession_T *session),
    int (*processerr)(SMFSettings_T *settings, SMFSession_T *session, int retval),
    int (*nhoperr)(SMFSettings_T *settings, SMFSession_T *session));

/** load all modules and run them */
int smf_modules_process(SMFProcessQueue_T *q, SMFSession_T *session, SMFSettings_T *settings);

/** deliver a message to the nexthop */
int smf_modules_deliver_nexthop(SMFSettings_T *settings, SMFProcessQueue_T *q, SMFSession_T *session);

/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSettings_T *settings, SMFSession_T *session, SMFList_T *initial_headers);

int smf_modules_engine_load(SMFSettings_T *settings);

int smf_modules_init(SMFSettings_T *settings, char *custom_libdir);
int smf_modules_unload(SMFSettings_T *settings);

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_MODULES_H */
