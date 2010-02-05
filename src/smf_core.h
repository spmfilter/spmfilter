/* 
 * file: smf_core.h
 * desc: interface to core routines used by smf
 * auth: Sebastian Jaekel <sj@space.net>
 */

#ifndef _SMF_CORE_H
#define	_SMF_CORE_H

#include <glib.h>

#include "spmfilter.h"

typedef int (*ModuleLoadFunction)(MailConn_T *mconn);

struct process_queue_ {
	GPtrArray *queue;
	int (*load_error)(void *args);
	int (*processing_error)(int retval, void *args);
};

typedef struct process_queue_  ProcessQueue_T;


/** initialize the process queue */
ProcessQueue_T *smf_core_pqueue_init(int(*loaderr)(void *args),
	int (*processerr)(int retval, void *args));

/** loads modules according to configuration and puts them into to
 *  array
 */
int smf_core_load_modules(ProcessQueue_T *q);

/** run the queue of loaded modules */
int smf_core_process_modules(ProcessQueue_T *q, MailConn_T *mconn);

#endif	/* _SMF_CORE_H */

