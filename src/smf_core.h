/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Sebastian Jaekel and SpaceNet AG
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

#ifndef _SMF_CORE_H
#define	_SMF_CORE_H

#include "spmfilter.h"

typedef int (*ModuleLoadFunction)(MailConn_T *mconn);

struct process_queue_ {
	int (*load_error)(void *args);
	int (*processing_error)(int retval, void *args);
	int (*nexthop_error)(void *args);
};

typedef struct process_queue_  ProcessQueue_T;


/** initialize the process queue */
ProcessQueue_T *smf_core_pqueue_init(int(*loaderr)(void *args),
	int (*processerr)(int retval, void *args),
	int (*nhoperr)(void *args));

/** load all modules and run them */
int smf_core_process_modules(ProcessQueue_T *q, MailConn_T *mconn);

/** deliver a message to the nexthop */
int smf_core_deliver_nexthop(ProcessQueue_T *q, MailConn_T *mconn);

/** sync header with queue file */
int smf_core_header_flush(MailConn_T *mconn);

#endif	/* _SMF_CORE_H */

