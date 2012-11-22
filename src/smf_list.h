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
 */

/*!
 * @file smf_list.h 
 * @brief Defines the SMFList_T data type and functions for a linked lists 
 *        that can be iterated over in both directions. 
 * @details Each element in the list contains a piece of data, together with 
 *        pointers which link to the previous and next elements in the list. 
 *        Using these pointers it is possible to move through the list in both directions.
 *
 * @details To create a new SMFList_T, use smf_list_new()
 * @details To insert an element into a SMFList_T, use smf_list_append()
 * @details To remove an element, use smf_list_remove()
 * @details To destroy a SMFList_T use smf_list_free()
 */

#ifndef _SMF_LIST_H
#define _SMF_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <cmime.h>

/*!
 * @typedef typedef CMimeListElem_T SMFListElem_T
 * @brief An element of a SMFList_T list
 */
typedef CMimeListElem_T SMFListElem_T;

/*!
 * @typedef typedef CMimeList_T SMFList_T
 * @brief Double linked list implementation
 */
typedef CMimeList_T SMFList_T;

/*!
 * @fn int smf_list_new(SMFList_T **list, void (*destroy)(void *data))
 * @brief Creates a new SMFList_T list
 * @param list out param to return the new list
 * @param destroy list destroy function
 * @returns 0 on success or -1 in case of error
 */
int smf_list_new(SMFList_T **list, void (*destroy)(void *data));

/*!
 * @fn int smf_list_free(SMFList_T *list)
 * @brief Free a SMFList_T list
 * @param list list to free
 * @returns 0 on success or -1 in case of error
 */
int smf_list_free(SMFList_T *list);

/*! 
 * @fn int smf_list_remove(SMFList_T *list, SMFListElem_T *elem, void **data)
 * @brief Remove an element from list
 * @param list a SMFList_T list
 * @param elem the SMFListElem_T element which should be removed
 * @param data out param to return element data
 * @returns 0 on success or -1 in case of error  
 */
int smf_list_remove(SMFList_T *list, SMFListElem_T *elem, void **data);

/*! 
 * @fn int smf_list_append(SMFList_T *list, void *data)
 * @brief Append data to the end of a list
 * @param list SMFList_T list to which new data should be appended
 * @param data new data which should be appended
 * @returns 0 on success or -1 in case of error 
 */
int smf_list_append(SMFList_T *list, void *data);

/*! 
 * @fn int smf_list_prepend(SMFList_T *list, void *data)
 * @brief Prepend data to a list
 * @param list a SMFList_T list to which new data should be prepended
 * @param data new data which should be appended
 * @returns 0 on success or -1 in case of error  
 */
int smf_list_prepend(SMFList_T *list, void *data);

/*! 
 * @fn void *smf_list_pop_tail(SMFList_T *list)
 * @brief Remove tail element from list and return data pointer
 * @param list a SMFList_T list
 * @returns data pointer of removed list element
 */
void *smf_list_pop_tail(SMFList_T *list);

/*! 
 * @fn void *smf_list_pop_head(SMFList_T *list);
 * @brief Remove head element from list an return data pointer
 * @param list a SMFList_T list
 * @returns data pointer of removed list element   
 */
void *smf_list_pop_head(SMFList_T *list);

/*! 
 * @fn int smf_list_insert_next(SMFList_T *list, SMFListElem_T *elem, void *data)
 * @brief Insert new element next to elem
 * @param list a SMFList_T list
 * @param elem a SMFListElem_T element
 * @param data data to insert next to element
 * @returns 0 on sucess or -1 in case of error
 */
int smf_list_insert_next(SMFList_T *list, SMFListElem_T *elem, void *data);

/*! 
 * @fn int smf_list_insert_prev(SMFList_T *list, SMFListElem_T *elem, void *data)
 * @brief Insert new element previous to elem 
 * @param list a SMFList_T list
 * @param elem a SMFListElem_T element
 * @param data data to insert previous to element
 * @returns 0 on sucess or -1 in case of error
 */
int smf_list_insert_prev(SMFList_T *list, SMFListElem_T *elem, void *data);

/*! 
 * @fn void smf_list_map(SMFList_T *list, void(*func)(SMFListElem_T *elem,void *args), void *args)
 * @brief Iterates over list and calls function for every element with the current element 
 * @param list a SMFList_T list
 * @param func function to call for each element
 * @param args optional arguments for function pointer
 */
void smf_list_map(SMFList_T *list, void(*func)(SMFListElem_T *elem,void *args), void *args);

/*! 
 * @fn int smf_list_map_new(SMFList_T *list, SMFList_T **new, void *(*func)(SMFListElem_T *elem, void *args), void *args)
 * @brief Iterates over list and calls function func with every element, return value of func will be saved in new list **new
 * @param list a SMFList_T list
 * @param new out param to return the new list
 * @param func function to call for each element
 * @param args optional arguments for function pointer
 * returns 0 on sucess or -1 in case of error
 */
int smf_list_map_new(SMFList_T *list, SMFList_T **new, void *(*func)(SMFListElem_T *elem, void *args), void *args);

/*!
 * @def smf_list_size(list)
 * @returns size of SMFList_T list
 */
#define smf_list_size(list) ((list)->size)

/*! 
 * @def smf_list_head(list)
 * @returns head element of SMFList_T list  
 */
#define smf_list_head(list) ((list)->head)

/*!
 * @def smf_list_tail(list)
 * @returns tail element of SMFList_T list
 */
#define smf_list_tail(list) ((list)->tail)

/*!
 * @def smf_list_is_head(elem)
 * @returns 1 if element is list head, 0 if not
 */
#define smf_list_is_head(elem) ((elem)->prev == NULL ? 1 : 0)

/*! 
 * @def smf_list_is_tail(elem)
 * @returns 1 if element is list tail, 0 if not
 */
#define smf_list_is_tail(elem) ((elem)->next == NULL ? 1 : 0)

/*!
 * @def smf_list_data(elem)
 * @returns data pointer of element
 */
#define smf_list_data(elem) ((elem)->data)

/*!
 * @def smf_list_next(elem)
 * @returns next element
 */
#define smf_list_next(elem) ((elem)->next)

/*!
 * @def smf_list_prev(elem)
 * @returns previous element
 */
#define smf_list_prev(elem) ((elem)->prev)

#ifdef __cplusplus
}
#endif

#endif  /* _SMF_LIST_H */