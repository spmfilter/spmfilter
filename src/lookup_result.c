/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include "lookup.h"
#include "spmfilter.h"

#define THIS_MODULE "lookup_result"

/** Allocates memory for LookupResult_T
 *
 * \returns newly allocated LookupResult_T
 */
LookupResult_T *lookup_result_new(void) {
	return (LookupResult_T *)g_ptr_array_new();
}

/** Adds a new element on to the end of the list.
 *
 * \param *l pointer to LookupResult_T
 * \param *elem_data pointer to LookupElement_T
 */
void lookup_result_add(LookupResult_T *l, LookupElement_T *elem_data) {
	g_ptr_array_add((GPtrArray *)l,elem_data);
}

/** Returns LookupElement_T at the given index of LookupResult_T
 *
 * \param l LookupResult_T list
 * \param i the index of the pointer to return
 *
 * \returns LookupElement_T at the given index
 */
LookupElement_T *lookup_result_index(LookupResult_T *l, int i) {
	return (LookupElement_T *)g_ptr_array_index((GPtrArray *)l,i);
}

/** Frees all of the memory used by LookupResult_T
 *
 * \param *l pointer to LookupResult_T
 */
void lookup_result_free(LookupResult_T *l) {
	int i;
	for (i = 0; i < l->len; i++) {
		g_hash_table_unref((GHashTable *)lookup_result_index(l,i));
	}
	g_free(l);
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
void lookup_element_add(LookupElement_T *e, char *key, void *value) {
	g_hash_table_insert((GHashTable *)e, key, value);
}

/** Looks up a key in LookupElement_T
 *
 * \param *e pointer to LookupElement_T
 * \param *key the key to look up
 *
 * \returns the associated value, or NULL if the key is not found
 */
void *lookup_element_get(LookupElement_T *e, char *key) {
	if (e == NULL)
		return NULL;
	return g_hash_table_lookup((GHashTable *)e,key);
}
