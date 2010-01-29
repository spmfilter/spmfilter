#ifndef _LOOKUP_RESULT_H
#define	_LOOKUP_RESULT_H

#include "spmfilter.h"

/** Creates a new element for LookupResult_T
 *
 * \returns pointer to new allocated LookupRow_T
 */
LookupElement_T *lookup_element_new(void);

/** Inserts a new key and value into LookupElement_T
 *  If the key already exists in LookupElement_T its current value is
 *  replaced with the new value.
 *
 * \param *e pointer to LookupElement_T
 * \param *key a key to insert
 * \param *value the value to associate with the key
 */
void lookup_row_insert(LookupElement_T *e, char *key, void *value);

#endif	/* _LOOKUP_RESULT_H */

