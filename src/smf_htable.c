/*
 * file: smf_htable.c
 * desc: a simple hashtable implementation which currently supports no
 *       dynamic resizing
 * auth: Sebastian Jaekel <sj@space.net>
 */

#include <stdio.h>
#include <stdlib.h>

#include "smf_clist.h"
#include "smf_htable.h"

/* create a hash for a given key
 *
 * taken from 'Mastering Algorithms with C' by Kyle Loudon (O'Reilly),
 * First Edition 1999, Page 146 pp
 */
int hash_key_old(const void *key) {
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

	printf("val %s, hash => %d\n", (char *)key,val);

	return(val % 4001);
}

/* used by ELF */
unsigned int hash_key(char *str) {

	unsigned int len  = strlen(str);
	unsigned int hash = 0;
	unsigned int x    = 0;
	unsigned int i    = 0;

	for(i = 0; i < len; str++, i++) {
		hash = (hash << 4) + (*str);
		if((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
		}
		hash &= ~x;
	}

	return hash % 4001;
}


/* create a new hash table */
HTABLE_T *htable_init(int buckets, void (*destroy)(void *data),
	int (*match)(const void *key, const void *key2))
{
	int bucks;
	int i;
	HTABLE_T *table;

	/* allocate memore */
	table = (HTABLE_T *)calloc(1,sizeof(HTABLE_T));

	/* use 0 to get the default amount of buckets */
	if(buckets == 0) {
		bucks = DEFAULT_BUCKETS;
	} else {
		bucks = buckets;
	}

	table->buckets = bucks;
	table->table = (DLIST_T *)malloc(bucks * sizeof(DLIST_T));

	if(table->table == NULL) {
		return(NULL);
	}


	for(i=0; i<bucks; i++) {
		table->table[i] = *(dlist_init(NULL));
	}

	table->match = match;
	table->destroy = destroy;
	table->size = 1;

	return(table);
}

/* insert an element into the table */
int htable_insert(HTABLE_T *table, void *key, void *value) {
	int bucket;
	int retval;
	HTABLE_PAIR_T *pair;

	/* TODO: update key instead of returning an error */
	if(htable_lookup(table, key) != NULL) {
		return(1);
	}

	/* allocate new pair */
	pair = (HTABLE_PAIR_T *)calloc(1, sizeof(HTABLE_PAIR_T));
	pair->key = key;
	pair->value = value;

	/* put into bucket */
	bucket = hash_key(key) % table->buckets;
	retval = dlist_push_back(&table->table[bucket],(void *)pair);

	if(retval == 0) {
		table->size++;
		return(0);
	} else {
		return(retval);
	}
}

/* lookup an element in the table */
void *htable_lookup(HTABLE_T *table, void *key) {
	DLIST_ELEM_T *elem;
	HTABLE_PAIR_T *pair;
	int bucket;

	bucket = hash_key(key) % table->buckets;

	for(elem = dlist_head(&table->table[bucket]);elem != NULL; elem = dlist_next(elem)) {
		if(table->match(key, ((HTABLE_PAIR_T *)elem->data)->key)) {
			pair = (HTABLE_PAIR_T *)elem->data;
			return(pair->value);
		}
	}

	return(NULL);
}

/* * * MATCH FUNCTIONS * * */
int htable_match_string(const void *key1, const void *key2) {
	if(strcmp((char *)key1, (char *)key2) == 0) {
		return(1);
	}

	return(0);
}