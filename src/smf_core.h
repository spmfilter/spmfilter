/* 
 * file: smf_core.h
 * desc: interface to core routines used by smf
 * auth: Sebastian Jaekel <sj@space.net>
 */

#ifndef _SMF_CORE_H
#define	_SMF_CORE_H

#include <glib.h>

struct process_queue_ {
	GPtrArray *queue;
	void (*errhandler)(void *args);
};

typedef struct process_queue_  ProcessQueue_T;


/** initialize the process queue */
ProcessQueue_T *smf_core_pqueue_init(void(*errhandler)(void *args));

/** loads modules according to configuration and puts them into to
 *  array
 */
int smf_core_load_modules(ProcessQueue_T *q);

#endif	/* _SMF_CORE_H */

