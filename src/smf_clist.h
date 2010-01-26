/*
 * file: smf_clist.h
 * desc: interface of doubly linked list
 * auth: Sebastian Jaekel <sj@space.net>
 */
#ifndef _SMF_CLIST_H
#define	_SMF_CLIST_H

/** a doubly linked list element */
struct dlist_elem_ {
	void *data;

	struct dlist_elem_ *prev;
	struct dlist_elem_ *next;
};

/* typedef for element */
typedef struct dlist_elem_ DLIST_ELEM_T;

/** a doubly linked list */
struct dlist_ {
	int size;

	void (*destroy)(void *data);

	DLIST_ELEM_T *head;
	DLIST_ELEM_T *tail;
};

/* typedef for doubly linked list */
typedef struct dlist_ DLIST_T;

/** initialize a list
 *
 * \param **list pointer to pointer to new list for which memory will be allocated
 * \param *destroy function pointer for cleaning up data element resources, optional,
 *   if not required, set to NULL
 * 
 * \returns -1 on error, 0 on success
 */
int dlist_init(DLIST_T **list, void (*destroy)(void *data));

/** destroy a complete list
 *
 * \param pointer to list to clean
 * 
 * \returns -1 on error, 0 on success
 */
int dlist_destroy(DLIST_T *list);

/** remove an element
 *
 * \param *list pointer to list
 * \param *elem pointer to element to remove
 * \param pointer to void pointer which holds elements data afer removing from
 *   list, needs to be cleaned manually
 * 
 * \returns -1 on error, 0 on success
 */
int dlist_remove(DLIST_T *list, DLIST_ELEM_T *elem, void **data);

/** appends an element to the end of the list
 *
 * \param *list pointer to list
 * \param *data void pointer to data element to use for new element
 * 
 * \returns -1 on error, 0 on success
 */
int dlist_push_back(DLIST_T *list, void *data);

/** prepend an element to the list
 *
 * \param *list pointer to list
 * \param *data void pointer to data element to use for new element
 *
 * \returns -1 on error, 0 on success
 */
int dlist_push_front(DLIST_T *list, void *data);

/** remove an element from the tail of the list
 *
 * \param *list pointer to list
 *
 * \returns void pointer to data element of removed element
 */
void *dlist_pop_back(DLIST_T *list);

/** removes and element from the head of the list
 * 
 * \param *list pointer to list
 *
 * \returns void pointer to data element of removed element
 */
void *dlist_pop_front(DLIST_T *list);

/** inserts new element next to elem
 *
 * \param *list pointer to list
 * \param *elem pointer to elem to which new element should be inserted next
 * \param *data void pointer to data element to use for new element
 *
 * \returns -1 on error, 0 on success
 */
int dlist_ins_next(DLIST_T *list, DLIST_ELEM_T *elem, void *data);

/** insert new element previous to elem
 *
 * \param *list pointer to list
 * \param *elem pointer to elem to which new element should be inserted before
 * \param *data void pointer to data element to use for new element
 *
 * \returns -1 on error, 0 on success
 */
int dlist_ins_prev(DLIST_T *list, DLIST_ELEM_T *elem, void *data);

/** iterates over list and calls function for every element with the current
 *  element
 *
 * \param *list pointer to list to iterate
 * \param *func function pointer to function to apply with signature of
 *   void func(DLIST_ELEM_T *elem, void *args)
 * \param *args void pointer of possible arguments to pass to *func
 * 
 * \returns nothing
 */
void dlist_map(DLIST_T *list, void(*func)(DLIST_ELEM_T *elem,void *args), void *args);

/** iterates over list and calls function func with every element, return value
 *  of func will be saved in new list **new
 *
 * \param *list pointer to list to iterate
 * \param *func function pointer to function to apply with signature of
 *   void *func(DLIST_ELEM_T *elem, void *args)
 * \param *args void pointer of possible arguments to pass to *func
 *
 * \returns pointer to list with results or NULL on error
 */
DLIST_T *dlist_map_new(DLIST_T *list, void *(*func)(DLIST_ELEM_T *elem,
	void *args), void *args);

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
