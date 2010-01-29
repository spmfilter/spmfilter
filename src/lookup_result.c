#include <glib.h>

#include "lookup.h"
#include "spmfilter.h"

#define THIS_MODULE "lookup_result"

/** Allocates memory for LookupResult_T
 *
 * \returns newly allocated LookupResult_T
 */
LookupResult_T *lookup_result_new(void) {
	LookupResult_T *l = (LookupResult_T *)g_slice_new(GList);
	return l;
}

/** Adds a new element on to the end of the list.
 *  The return value is the new start of the list,
 *  which may have changed, so make sure you store the new value.
 *
 * \param *l pointer to LookupResult_T
 * \param *elem_data pointer to LookupElement_T
 *
 * \returns the new start of LookupResult_T
 */
LookupResult_T *lookup_result_append(LookupResult_T *l, LookupElement_T *elem_data) {
	return (LookupResult_T *) g_list_append((GList *)l,elem_data);
}

/** Get the first element of LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 *
 * \returns the first element in LookupResult_T,
 *          or NULL if there are no elements
 */
LookupResult_T *lookup_result_first(LookupResult_T *l) {
	return (LookupResult_T *)g_list_first((GList *)l);
}

/** Get the last element of LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 *
 * \returns the first element in LookupResult_T,
 *          or NULL if there are no elements
 */
LookupResult_T *lookup_result_last(LookupResult_T *l) {
	return (LookupResult_T *)g_list_last((GList *)l);
}

/** Get the previous element of LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 *
 * \returns the previous element, or NULL if there are no previous elements.
 */
LookupResult_T *lookup_result_previous(LookupResult_T *l) {
	return (LookupResult_T *)g_list_previous((GList *)l);
}

/** Get the next element of LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 *
 * \returns the next element, or NULL if there is no following element
 */
LookupResult_T *lookup_result_next(LookupResult_T *l) {
	return (LookupResult_T *)g_list_next((GList *)l);
}

/** Get the element at the given position
 *
 * \param *l pointer to LookupResult_T
 * \param i the position of the element
 *
 * \returns the element, or NULL if the position is off the end
 */
LookupResult_T *lookup_result_nth(LookupResult_T *l, int i) {
	return (LookupResult_T *)g_list_nth((GList *)l,(guint)i);
}

/** This function iterates over the whole list to count its elements.
 *
 * \param *l pointer to LookupResult_T
 *
 * \returns the number of elements
 */
int lookup_result_count(LookupResult_T *l) {
	return g_list_length((GList *)l);
}

/** Frees all of the memory used by LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 */
void lookup_result_free(LookupResult_T *l) {
	for (l = lookup_result_first(l); l; l = lookup_result_next(l)) {
		g_hash_table_unref(l->elem->data);
		l = lookup_result_next(l);
	}
	g_list_free((GList *)l);
}

/** Creates a new element for LookupResult_T
 *
 * \returns pointer to new allocated LookupElement_T
 */
LookupElement_T *lookup_element_new(void) {
	return (LookupElement_T *)g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
}

/** Inserts a new key and value into LookupElement_T
 *  If the key already exists in LookupElement_T its current value is
 *  replaced with the new value.
 * 
 * \param *e pointer to LookupElement_T
 * \param *key a key to insert
 * \param *value the value to associate with the key
 */
void lookup_element_insert(LookupElement_T *e, char *key, void *value) {
	g_hash_table_insert((GHashTable *)e, key, value);
}

/** Looks up a key in LookupResult_T
 *
 * \param *r pointer to LookupResult_T
 * \param *key the key to look up
 *
 * \returns the associated value, or NULL if the key is not found
 */
void *lookup_get_element(LookupResult_T *r, char *key) {
	return g_hash_table_lookup((GHashTable *)r->elem->data,key);
}
