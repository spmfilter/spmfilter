/*
 * test program
 */

#include <stdio.h>
#include <stdlib.h>

#include "smf_clist.h"
#include "smf_htable.h"


void printme(DLIST_ELEM_T *elem, void *args) {
	printf("elem data => %s\n", (char *)elem->data);
}

void print_int(DLIST_ELEM_T *elem, void *args) {
	printf("elem data => %d\n", (int)elem->data);
}

void *apply_double(DLIST_ELEM_T *elem, void *args) {
	int res;
	res = (int *)elem->data;
	return((void *)(res * 2));
}


int main(void) {
	int i;
	int bytes;
	HTABLE_T *tab;
	FILE *words;
	char buffer[128];
	char *c;

#if 0
	DLIST_T *list;
	DLIST_T *ints;
	DLIST_T *int2;
	DLIST_ELEM_T *elem;
	void *data;

	int eins = 1;
	int zwei = 2;
	int drei = 3;

	char t1[] = "test1";
	char t2[] = "t2";
	char t3[] = "test3";
	char t4[] = "tt4";
	char t5[] = "55";
	char t6[] = "i am new first!";

	list = dlist_init(NULL);
	dlist_push_back(list, (void *) t1);
	dlist_push_back(list, (void *) t2);
	dlist_push_back(list, (void *) t3);
	dlist_push_back(list, (void *) t4);
	dlist_push_back(list, (void *) t5);

	printf("first list size is %d\n", dlist_size(list));

	elem = dlist_head(list);
	while(elem != NULL) {
		printf("current value is %s\n", (char *)elem->data);

		if(strcmp((char *)elem->data, "test3") == 0) {
			dlist_remove(list, elem, &data);
		}

		elem = elem->next;
	}

	printf("second list size is %d\n", dlist_size(list));
	elem = dlist_head(list);
	while(elem != NULL) {
		printf("current value is %s\n", (char *)elem->data);
		elem = elem->next;
	}

	printf("\n");
	dlist_push_front(list, (void *)t6);
	elem = dlist_head(list);
	while(elem != NULL) {
		printf("current value is %s\n", (char *)elem->data);
		elem = elem->next;
	}
	printf("\n");

	data = dlist_pop_back(list);
	printf("pop back returned %s\n", (char *)data);
	elem = dlist_head(list);
	while(elem != NULL) {
		printf("current value is %s\n", (char *)elem->data);
		elem = elem->next;
	}
	printf("\n");

	data = dlist_pop_front(list);
	printf("pop back returned %s\n", (char *)data);
	dlist_map(list,printme,NULL);
	printf("\n");


	dlist_destroy(list);


	printf("\n---integer list---\n");
	ints = dlist_init(NULL);

	dlist_push_back(ints, (void *)eins);
	dlist_push_back(ints, (void *)zwei);
	dlist_push_back(ints, (void *)drei);

	dlist_map(ints,print_int,NULL);
	int2 = dlist_map_new(ints,apply_double,NULL);
	dlist_destroy(ints);
	dlist_map(int2,print_int,NULL);
#endif
	tab = htable_init(0,NULL,htable_match_string);
	words = fopen("my_words","r");

	i = 0;
	while(fgets(buffer,128,words) != NULL) {
		buffer[strlen(buffer) - 1] = '\0';
		char *c = (char *)calloc(strlen(buffer) + 1,sizeof(char));
		strncpy(c,buffer,strlen(buffer));
		htable_insert(tab, (void *)c,(void *)"ein teststring");
		i++;
	}

	int total=0;
	for(i=0; i<tab->buckets;i++) {
		printf("%d,%d\n", i, tab->table[i].size);
	}

	

	return 0;
}
