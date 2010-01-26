/*
 * file: smf_clist.h
 * desc: interface of doubly linked list
 * auth: Sebastian Jaekel <sj@space.net>
 */
#ifndef _SMF_CLIST_H
#define	_SMF_CLIST_H


struct dlist_elem_ {
	void *data;

	struct dlist_elem_ *prev;
	struct dlist_elem_ *next;
};

typedef struct dlist_elem_ DLIST_ELEM_T;

struct dlist_ {
	int size;

	void (*destroy)(void *data);

	DLIST_ELEM_T *head;
	DLIST_ELEM_T *tail;
};

typedef struct dlist_ DLIST_T;

/** initialize a list
 */
int dlist_init(DLIST_T **list, void (*destroy)(void *data));

/** destroy a complete list
 */
int dlist_destroy(DLIST_T *list);

/** remove an element
 */
int dlist_remove(DLIST_T *list, DLIST_ELEM_T *elem, void **data);

/** append to list
 */
int dlist_push_back(DLIST_T *list, void *data);

/** prepend to list
 */
int dlist_push_front(DLIST_T *list, void *data);

/** remove from tail
 */
void *dlist_pop_back(DLIST_T *list);

/** remove from front
 */
void *dlist_pop_front(DLIST_T *list);

/** insert new element next to elem
 */
int dlist_ins_next(DLIST_T *list, DLIST_ELEM_T *elem, void *data);

/** insert new element previous to elem
 */
int dlist_ins_prev(DLIST_T *list, DLIST_ELEM_T *elem, void *data);

/* * * MACROS * * */
#define dlist_size(list) ((list)->size)

#define dlist_head(list) ((list)->head)
#define dlist_tail(list) ((list)->tail)

#define dlist_is_head(elem) ((elem)->prev == NULL ? 1 : 0)
#define dlist_is_tail(elem) ((elem)->next == NULL ? 1 : 0)

#define dlist_data(elem) ((elem)->data)

#define dlist_next(elem) ((elem)->next)
#define dlist_prev(elem) ((elem)->prev)

#endif	/* _SMF_CLIST_H */
