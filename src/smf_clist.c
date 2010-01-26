/*
 * file: smf_clist.c
 * desc: implementation of doubly linked list
 * auth: Sebastian Jaekel <sj@space.net>
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "smf_clist.h"

/* initializes a new empty doubly linked list */
int dlist_init(DLIST_T **list, void (*destroy)(void *data)) {
	(*list) = (DLIST_T *)calloc(1,sizeof(DLIST_T));

	if(*list == NULL) {
		return(-1);
	}

	(*list)->size = 0;
	(*list)->head = NULL;
	(*list)->tail = NULL;
	(*list)->destroy = destroy;

	return(0);
}

/* destroy a complete list */
int dlist_destroy(DLIST_T *list) {
	void *data;

	while(dlist_size(list) > 0) {
		if(dlist_remove(list, dlist_tail(list), (void **)&data) == 0 &&
			list->destroy != NULL) {

			list->destroy(data);
		}
	}

	/* no more operations please... */
	memset(list,0,sizeof(DLIST_T));

	return(0);
}

/* remove an element from the list */
int dlist_remove(DLIST_T *list, DLIST_ELEM_T *elem, void **data) {
	/* no null element and no empty list */
	if(elem == NULL || dlist_size(list) == 0) {
		return(-1);
	}

	*data = elem->data;

	/* handle removal of first element */
	if(elem == dlist_head(list)) {
		list->head = elem->next;

		if(list->head == NULL) {
			list->tail = NULL;
		} else {
			list->head->prev = NULL;
		}
	} else {
		elem->prev->next = elem->next;

		if(elem->next == NULL) {
			list->tail = elem->prev;
		} else {
			elem->next->prev = elem->prev;
		}
	}

	free(elem);
	list->size--;

	return(0);
}

/* remove tail element and return data pointer */
void* dlist_pop_back(DLIST_T *list) {
	void *data;
	int ret;

	ret = dlist_remove(list,dlist_tail(list),&data);

	if(ret == 0) {
		return(data);
	} else {
		return(NULL);
	}
}

void *dlist_pop_front(DLIST_T *list) {
	void *data;
	int ret;

	ret = dlist_remove(list,dlist_head(list),&data);

	if(ret == 0) {
		return(data);
	} else {
		return(NULL);
	}
}

/* insert new elem next to given element elem */
int dlist_ins_next(DLIST_T *list, DLIST_ELEM_T *elem, void *data) {
	DLIST_ELEM_T *new = (DLIST_ELEM_T *)calloc(1,sizeof(DLIST_ELEM_T));
	if(new == NULL) {
		return(-1);
	}
	
	/* no null element if list not empty */
	if(elem == NULL && dlist_size(list) != 0) {
		return(-1);
	}

	new->data = data;

	if(dlist_size(list) == 0) {
		list->head = new;
		list->tail = new;
		new->next = NULL;
		new->prev = NULL;
	} else {
		new->next = elem->next;
		new->prev = elem;

		if(elem->next == NULL) {
			list->tail = new;
		} else {
			elem->next->prev = new;
		}

		elem->next = new;
	}

	list->size++;

	return(0);
}

/* insert new element previous to given element elem */
int dlist_ins_prev(DLIST_T *list, DLIST_ELEM_T *elem, void *data) {
	DLIST_ELEM_T *new = (DLIST_ELEM_T *)calloc(1,sizeof(DLIST_ELEM_T));
	if(new == NULL) {
		return(-1);
	}

	/* no null element if list not empty */
	if(elem == NULL && dlist_size(list) != 0) {
		return(-1);
	}

	new->data = data;

	if(dlist_size(list) == 0) {
		list->head = new;
		list->tail = new;
		new->next = NULL;
		new->prev = NULL;
	} else {
		new->next = elem;
		new->prev = elem->prev;

		if(elem->prev == NULL) {
			list->head = new;
		} else {
			elem->prev->next = new;
		}

		elem->prev = new;
	}

	list->size++;

	return(0);
}

/* append to the end of a list */
int dlist_push_back(DLIST_T *list, void *data) {
	return dlist_ins_next(list,dlist_tail(list),data);
}

/* prepend an element to the list */
int dlist_push_front(DLIST_T *list, void *data) {
	return dlist_ins_prev(list,dlist_head(list),data);
}

/* apply function func to every element in the list */
void dlist_map(DLIST_T *list, void(*func)(DLIST_ELEM_T *elem,void *args), void *args) {
	DLIST_ELEM_T *elem;

	elem = dlist_head(list);
	while(elem != NULL) {
		func(elem,args);
		elem = elem->next;
	}
}


DLIST_T *dlist_map_new(DLIST_T *list, void *(*func)(DLIST_ELEM_T *elem,
	void *args), void *args)
{
	DLIST_T *new;
	DLIST_ELEM_T *elem;
	int ret;

	ret = dlist_init(&new, NULL);
	if(ret != 0) {
		return(NULL);
	}

	elem = dlist_head(list);
	while(elem != NULL) {
		dlist_push_back(new,func(elem,args));
		elem = elem->next;
	}

	return(new);
}
