#/* spmfilter - mail filtering framework
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
 * @brief Implements a dictionary for string variables.
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
 * @brief Create a new SMFDict_T object.
 * @return a newly allocated SMFDict_T objet.
 */
SMFDict_T *smf_dict_new(void);

/*!
 * @fn void smf_dict_free(SMFDict_T *dict)
 * @brief Free a SMFDict_T object
 * @param dict SMFDict_T object to deallocate.
 */
void smf_dict_free(SMFDict_T *dict);

/*!
 * @fn int smf_dict_set(SMFDict_T *dict, const char * key, const char * val)
 * @brief Set a value in a dictionary.
 * @param dict a SMFDict_T object to modify.
 * @param key Key to modify or add.
 * @param val Value to add.
 * @return 0 on success or -1 in case of error
 *
 * If the given key is found in the dictionary, the associated value is
 * replaced by the provided one. If the key cannot be found in the
 * dictionary, it is added to it.
 */
int smf_dict_set(SMFDict_T *dict, const char * key, const char * val);

/*!
 * @fn char *smf_dict_get(SMFDict_T *dict, const char * key)
 * @brief Get a value from a dictionary.
 * @param dict a SMFDict_T object
 * @param key Key to look for in the dictionary.
 * @param def Default value to return if key not found.
 * @return value of key
 */
char *smf_dict_get(SMFDict_T *dict, const char * key);

/*!
 * @fn void smf_dict_remove(SMFDict_T *dict, const char * key)
 * @brief Remove a key from dictionrary
 * @param dict a SMFDict_T object to modify.
 * @param key Key to remove.
 */
void smf_dict_remove(SMFDict_T *dict, const char * key);

/*!
 * @fn SMFList_T *smf_dict_get_keys(SMFDict_T *dict)
 * @brief Get list with keys stored in dictionary
 * @param dict a SMFDict_T  object
 * @return a SMFList_T object
 */
SMFList_T *smf_dict_get_keys(SMFDict_T *dict);

/*! 
 * @fn void smf_dict_map(SMFDict_T *dict, void(*func)(char *key,char *value, void *args), void *args)
 * @brief Iterates over dict and calls function for every key/value pair  
 * @param dict a SMFDict_T object
 * @param func function to call for each element
 * @param args optional arguments for function pointer
 */
void smf_dict_map(SMFDict_T *dict, void(*func)(char *key,char *value, void *args), void *args);

/*!
 * @def smf_dict_count(dict)
 * @return number of elemtents in a SMFDict_T 
 */
#define smf_dict_count(dict) ((dict)->n)

#endif  /* _SMF_DICT_H */
