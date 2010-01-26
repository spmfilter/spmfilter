/*
 * file: smf_htable.c
 * desc: hashtable interface
 * auth: Sebastian Jaekel <sj@space.net>
 */

#ifndef _SMG_HTABLE_H
#define	_SMG_HTABLE_H

#include "smf_clist.h"

enum {
	PRIME_TABLESIZE = 3659,
	DEFAULT_BUCKETS = 4096
};


/** structure for chained hash table */
struct htable_ {
	int buckets;

	void (*destroy)(void *data);
	int (*match)(const void *key, const void *key2);

	int size;
	DLIST_T *table;
};

/* hashtable typedef */
typedef struct htable_ HTABLE_T;

/** create a new table */
int htable_init(HTABLE_T **table, int buckets, void (*destroy)(void *data));

/** destroy a table */
int htable_destroy(HTABLE_T *table);

/** insert an element into the table */
int htable_insert(HTABLE_T *table, void *data);

/** remove an element from the table */
int htable_remove(HTABLE_T *table, void **data);

/** lookup an element */
int htable_lookup(HTABLE_T *table, void **data);

#endif	/* _SMG_HTABLE_H */
