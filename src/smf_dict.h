/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
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
 *
 * Original implementation by N.Devillard
 */

/*!
 * @file smf_dict.h 
 * @brief Defines the SMFDict_T data type and functions for a dictionary.
 * @details A SMFDict_T provides associations between keys and values so that given a key, the associated 
 *          value can be found very quickly. Please note that the keys and values are copied when inserted 
 *          into the dictionary and will be freed with smf_dict_free()
 *
 * @details To create a new SMFDict_T, use smf_dict_new()
 * @details To insert a key and value into a SMFDict_T, use smf_dict_set()
 * @details To lookup a value corresponding to a given key, use smf_dict_get()
 * @details To remove a key and value, use smf_dict_remove()
 * @details To call a function for each key and value pair use smf_dict_map() 
 * @details To destroy a SMFDict_T use smf_dict_free()
 */

#ifndef _SMF_DICT_H
#define _SMF_DICT_H

#include "smf_list.h"

typedef struct {
    int n; /**< number of entries in dictionary */
    int size; /**< storage size */
    char **val; /**< list of values */
    char **key; /**< list of string keys */
    unsigned *hash; /**< list of hash values for keys */
} SMFDict_T ;

/*!
 * @fn SMFDict_T *smf_dict_new(void);
 * @brief Create a new SMFDict_T.
 * @return a newly allocated SMFDict_T.
 */
SMFDict_T *smf_dict_new(void);

/*!
 * @fn void smf_dict_free(SMFDict_T *dict)
 * @brief Frees a SMFDict_T with all keys and values
 * @param dict SMFDict_T to deallocate.
 */
void smf_dict_free(SMFDict_T *dict);

/*!
 * @fn int smf_dict_set(SMFDict_T *dict, const char *key, const char *val)
 * @brief Inserts a key and value into a SMFDict_T
 * @details If the given key is found in the dictionary, the associated value is
 *          replaced by the provided one. If the key cannot be found in the
 *          dictionary, it will be added to it.
 * @param dict a SMFDict_T object to modify.
 * @param key key to modify or add.
 * @param val Value to add.
 * @return 0 on success or -1 in case of error
 */
int smf_dict_set(SMFDict_T *dict, const char * key, const char * val);

/*!
 * @fn char *smf_dict_get(SMFDict_T *dict, const char * key)
 * @brief Looks up a key in a SMFDict_T and get the associated value
 * @param dict a SMFDict_T object
 * @param key key to look for in the dictionary.
 * @return the associated value, or NULL if the key is not found
 */
char *smf_dict_get(SMFDict_T *dict, const char * key);

/*!
 * @fn void smf_dict_remove(SMFDict_T *dict, const char * key)
 * @brief Removes a key and its associated value from a SMFDict_T
 * @param dict a SMFDict_T 
 * @param key the key to remove.
 */
void smf_dict_remove(SMFDict_T *dict, const char * key);

/*!
 * @fn SMFList_T *smf_dict_get_keys(SMFDict_T *dict)
 * @brief Retrieves every key inside a SMFDict_T
 * @param dict a SMFDict_T  object
 * @return a SMFList_T containing all the keys inside the dictionary.
 *         Use smf_list_free() when done using the list.
 */
SMFList_T *smf_dict_get_keys(SMFDict_T *dict);

/*! 
 * @fn void smf_dict_map(SMFDict_T *dict, void(*func)(char *key,char *value, void *args), void *args)
 * @brief Calls the given function for each of the key/value pairs in the SMFDict_T. The function is 
 *        passed the key and value of each pair, and the given args parameter. 
 * @param dict a SMFDict_T object
 * @param func function to call for each element
 * @param args optional arguments to pass to the function
 */
void smf_dict_map(SMFDict_T *dict, void(*func)(char *key,char *value, void *args), void *args);

/*!
 * @def smf_dict_count(dict)
 * @return number of elemtents in a SMFDict_T 
 */
#define smf_dict_count(dict) ((dict)->n)

#endif  /* _SMF_DICT_H */
