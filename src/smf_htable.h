/*
 * file: smf_htable.c
 * desc: hashtable interface
 * auth: Sebastian Jaekel <sj@space.net>
 */

#ifndef _SMG_HTABLE_H
#define	_SMG_HTABLE_H

#include "smf_clist.h"

enum {
	PRIME_TABLESIZE = 4001,
	DEFAULT_BUCKETS = 4001
};


/** a hash table pair */
struct pair_ {
	void *key;
	void *value;
};

typedef struct pair_ HTABLE_PAIR_T;

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
HTABLE_T *htable_init(int buckets, void (*destroy)(void *data),
	int (*match)(const void *key, const void *key2));

/** destroy a table */
int htable_destroy(HTABLE_T *table);

/** insert an element into the table */
int htable_insert(HTABLE_T *table, void *key, void *value);

/** remove an element from the table */
int htable_remove(HTABLE_T *table, void *key);

/** lookup an element */
void *htable_lookup(HTABLE_T *table, void *key);

/* * * MATCH FUNCTION * * */
int htable_match_string(const void *key1, const void *key2);

#endif	/* _SMG_HTABLE_H */
