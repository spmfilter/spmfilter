/*
 * file: smf_htable.c
 * desc: a simple hashtable implementation which currently supports no
 *       dynamic resizing
 * auth: Sebastian Jaekel <sj@space.net>
 */

#include <stdlib.h>

#include "smf_clist.h"
#include "smf_htable.h"

/* create a hash for a given key
 *
 * taken from 'Mastering Algorithms with C' by Kyle Loudon (O'Reilly),
 * First Edition 1999, Page 146 pp
 */
int hash_key(const void *key) {
	const char *ptr;
	int val;

	val = 0;
	ptr = key;

	while(*ptr != '\0') {
		int tmp;

		val = (val << 4) + (*ptr);
		if(tmp = (val & 0xf0000000)) {
			val = val ^ (tmp >> 24);
			val = val ^ tmp;
		}

		ptr++;
	}

	return(val % PRIME_TABLESIZE);
}

/* create a new hash table */
int htable_init(HTABLE_T **table, int buckets, void (*destroy)(void *data),
	int (*match)(const void *key, const void *key2))
{
	int bucks;
	int i;

	/* use 0 to get the default amount of buckets */
	if(buckets == 0) {
		bucks = DEFAULT_BUCKETS;
	} else {
		bucks = buckets;
	}

	(*table)->buckets = bucks;
	(*table)->table = (DLIST_T *)calloc(bucks,sizeof(DLIST_T));

	if((*table)->table == NULL) {
		return(-1);
	}

	for(i=0; i<bucks; i++) {
		dlist_init(&(*table)->table[i],NULL);
	}

	(*table)->match = match;
	(*table)->destroy = destroy;
	(*table)->size = 1;

	return(0);
}

/* insert an element into the table */
int htable_insert(HTABLE_T *table, void *data) {
	void *temp;
	int bucket;
	int retval;

	temp = (void *)data;
	if(htable_lookup(table, &temo) == 0) {
		return(1);
	}

	


}

/* lookup an element in the table */
int htable_lookup(HTABLE_T *table, void **data) {
	DLIST_ELEM_T *elem;
	int bucket;

	bucket = hash_key(*data) % table->buckets;

	for(elem = dlist_head(&table->table[bucket]);elem != NULL; elem = dlist_next(elem)) {
		if(table->match(*data, elem->data)) {
			*data = elem->data;
			return(0);
		}
	}

	return(-1);
}


