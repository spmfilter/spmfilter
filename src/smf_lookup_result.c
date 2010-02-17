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

#include "smf_lookup.h"

#define THIS_MODULE "lookup_result"

/** Allocates memory for SMFLookupResult_T
 *
 * \returns newly allocated SMFLookupResult_T
 */
SMFLookupResult_T *smf_lookup_result_new(void) {
	return (SMFLookupResult_T *)g_ptr_array_new();
}

/** Adds a new element on to the end of the list.
 *
 * \param *l pointer to SMFLookupResult_T
 * \param *elem_data pointer to SMFLookupElement_T
 */
void smf_lookup_result_add(SMFLookupResult_T *l, SMFLookupElement_T *elem_data) {
	g_ptr_array_add((GPtrArray *)l,elem_data);
}

/** Returns SMFLookupElement_T at the given index of SMFLookupResult_T
 *
 * \param l SMFLookupResult_T list
 * \param i the index of the pointer to return
 *
 * \returns SMFLookupElement_T at the given index
 */
SMFLookupElement_T *smf_lookup_result_index(SMFLookupResult_T *l, int i) {
	return (SMFLookupElement_T *)g_ptr_array_index((GPtrArray *)l,i);
}

/** Frees all of the memory used by SMFLookupResult_T
 *
 * \param *l pointer to SMFLookupResult_T
 */
void smf_lookup_result_free(SMFLookupResult_T *l) {
	int i;
	for (i = 0; i < l->len; i++) {
		g_hash_table_unref((GHashTable *)smf_lookup_result_index(l,i));
	}
	g_free(l);
}


/** Creates a new element for SMFLookupResult_T
 *
 * \returns pointer to new allocated SMFLookupElement_T
 */
SMFLookupElement_T *smf_lookup_element_new(void) {
	return (SMFLookupElement_T *)g_hash_table_new((GHashFunc)g_str_hash,(GEqualFunc)g_str_equal);
}

/** Inserts a new key and value into SMFLookupElement_T
 *  If the key already exists in SMFLookupElement_T its current value is
 *  replaced with the new value.
 * 
 * \param *e pointer to SMFLookupElement_T
 * \param *key a key to insert
 * \param *value the value to associate with the key
 */
void smf_lookup_element_add(SMFLookupElement_T *e, char *key, void *value) {
	g_hash_table_insert((GHashTable *)e, key, value);
}

/** Looks up a key in SMFLookupElement_T
 *
 * \param *e pointer to SMFLookupElement_T
 * \param *key the key to look up
 *
 * \returns the associated value, or NULL if the key is not found
 */
void *smf_lookup_element_get(SMFLookupElement_T *e, char *key) {
	if (e == NULL)
		return NULL;
	return g_hash_table_lookup((GHashTable *)e,key);
}
