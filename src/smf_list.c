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

#include "smf_list.h"

 /* initializes a new empty doubly linked list */
int smf_list_new(SMFList_T **list, void (*destroy)(void *data)) {
    return cmime_list_new((CMimeList_T **)list,destroy);
}

/* destroy a complete list */
int smf_list_free(SMFList_T *list) {
    return cmime_list_free((CMimeList_T *)list);
}

/* remove an element from the list */
int smf_list_remove(SMFList_T *list, SMFListElem_T *elem, void **data) {
    return cmime_list_remove((CMimeList_T *)list, (CMimeListElem_T *)elem,data);
}

/* remove tail element and return data pointer */
void* smf_list_pop_tail(SMFList_T *list) {
    return cmime_list_pop_tail((CMimeList_T *)list);
}

void *smf_list_pop_head(SMFList_T *list) {
    return cmime_list_pop_head((CMimeList_T *)list);
}

/* insert new elem next to given element elem */
int smf_list_insert_next(SMFList_T *list, SMFListElem_T *elem, void *data) {
    return cmime_list_insert_next((CMimeList_T *)list, (CMimeListElem_T *)elem,data);
}

/* insert new element previous to given element elem */
int smf_list_insert_prev(SMFList_T *list, SMFListElem_T *elem, void *data) {
    return cmime_list_insert_prev((CMimeList_T *)list, (CMimeListElem_T *)elem,data);
}

/* append to the end of a list */
int smf_list_append(SMFList_T *list, void *data) {
    return cmime_list_append((CMimeList_T *)list,data);
}
 
/* prepend an element to the list */
int smf_list_prepend(SMFList_T *list, void *data) {
    return cmime_list_prepend((CMimeList_T *)list,data);
}

/* apply function func to every element in the list */
void smf_list_map(SMFList_T *list, void(*func)(SMFListElem_T *elem,void *args), void *args) {
    cmime_list_map((CMimeList_T *)list,func,args);
}

int smf_list_map_new(SMFList_T *list, SMFList_T **new, void *(*func)(SMFListElem_T *elem,
        void *args), void *args) {
    return smf_list_map_new((CMimeList_T *)list,(CMimeList_T **)new,func,args);
}
