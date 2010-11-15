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

#ifndef _SMF_MODULES_H
#define	_SMF_MODULES_H

#include "smf_session.h"
#include "smf_settings.h"

typedef int (*ModuleLoadFunction)(SMFSession_T *session);
typedef int (*LoadEngine) (SMFSettings_T *settings, int fd);

struct process_queue_ {
	int (*load_error)(void *args);
	int (*processing_error)(int retval, void *args);
	int (*nexthop_error)(void *args);
};

typedef struct process_queue_  ProcessQueue_T;


/** initialize the process queue */
ProcessQueue_T *smf_modules_pqueue_init(int(*loaderr)(void *args),
	int (*processerr)(int retval, void *args),
	int (*nhoperr)(void *args));

/** load all modules and run them */
int smf_modules_process(ProcessQueue_T *q, SMFSession_T *session, SMFSettings_T *settings);

/** deliver a message to the nexthop */
int smf_modules_deliver_nexthop(ProcessQueue_T *q, SMFSession_T *session);

/** Flush modified message headers to queue file */
int smf_modules_flush_dirty(SMFSession_T *session);

int smf_modules_engine_load(SMFSettings_T *settings, int fd);
#endif	/* _SMF_ODULES_H */

