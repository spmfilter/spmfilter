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

#define THIS_MODULE "dict"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "smf_internal.h"
#include "smf_dict.h"
#include "smf_list.h"

/* Doubles the allocated size associated to a pointer */
/* 'size' is the current allocated size. */
static void *_mem_double(void * ptr, int size) {
    void * newptr ;
 
    newptr = calloc(2*size, 1);
    if (newptr==NULL) {
        return NULL ;
    }
    memcpy(newptr, ptr, size);
    free(ptr);
    return newptr ;
}

unsigned _dict_hash(const char * key) {
    int len;
    unsigned hash;
    int i;

    len = strlen(key);
    for (hash=0, i=0 ; i<len ; i++) {
        hash += (unsigned)key[i] ;
        hash += (hash<<10);
        hash ^= (hash>>6) ;
    }
    hash += (hash <<3);
    hash ^= (hash >>11);
    hash += (hash <<15);
    return hash;
}

SMFDict_T *smf_dict_new(void) {
    SMFDict_T *dict = NULL;

    if (!(dict = (SMFDict_T *)calloc(1, sizeof(SMFDict_T))))
        return NULL;

    dict->size = 128;
    dict->val  = (char **)calloc(128, sizeof(char*));
    dict->key  = (char **)calloc(128, sizeof(char*));
    dict->hash = (unsigned int *)calloc(128, sizeof(unsigned));
    return dict;
}

void smf_dict_free(SMFDict_T *dict) {
    int i ;

    assert(dict);
    
    for (i=0; i<dict->size; i++) {
        if (dict->key[i] != NULL)
            free(dict->key[i]);
        if (dict->val[i] != NULL)
            free(dict->val[i]);
    }

    free(dict->val);
    free(dict->key);
    free(dict->hash);
    free(dict);
}

int smf_dict_set(SMFDict_T *dict, const char * key, const char * val) {
    int i;
    unsigned hash;

    assert(dict);
    assert(key);
    assert(val);
    
    /* Compute hash for this key */
    hash = _dict_hash(key);

    /* Find if value is already in dictionary */
    if (dict->n>0) {
        for (i = 0; i < dict->size; i++) {
            if (dict->key[i] == NULL)
                continue;
            if (hash == dict->hash[i]) { /* Same hash value */
                if (!strcmp(key, dict->key[i])) {   /* Same key */
                    /* Found a value: modify and return */
                    if (dict->val[i]!=NULL)
                        free(dict->val[i]);
                    dict->val[i] = val ? strdup(val) : NULL;
                    /* Value has been modified: return */
                    return 0;
                }
            }
        }
    }

    /* Add a new value */
    /* See if dictionary needs to grow */
    if (dict->n == dict->size) {
        /* Reached maximum size: reallocate dictionary */
        dict->val = (char **)_mem_double(dict->val,  dict->size * sizeof(char*));
        dict->key = (char **)_mem_double(dict->key,  dict->size * sizeof(char*)) ;
        dict->hash = (unsigned int *)_mem_double(dict->hash, dict->size * sizeof(unsigned)) ;
        if ((dict->val == NULL) || (dict->key == NULL) || (dict->hash == NULL)) {
            /* Cannot grow dictionary */
            return -1;
        }
        /* Double size */
        dict->size *= 2 ;
    }

    /* Insert key in the first empty slot. Start at d->n and wrap at
       d->size. Because d->n < d->size this will necessarily
       terminate. */
    for (i = dict->n; dict->key[i]; ) {
        if(++i == dict->size) i = 0;
    }
    /* Copy key */
    dict->key[i]  = strdup(key);
    dict->val[i]  = val ? strdup(val) : NULL ;
    dict->hash[i] = hash;
    dict->n ++;
    return 0;
}

char *smf_dict_get(SMFDict_T *dict, const char * key) {
    unsigned hash;
    int i;

    assert(dict);
    assert(key);

    hash = _dict_hash(key);
    for (i = 0; i < dict->size; i++) {
        if (dict->key[i] == NULL)
            continue ;
        /* Compare hash */
        if (hash == dict->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, dict->key[i])) {
                return dict->val[i] ;
            }
        }
    }
    return NULL;
}

void smf_dict_remove(SMFDict_T *dict, const char * key) {
    unsigned hash;
    int i;

    assert(dict);
    assert(key);

    hash = _dict_hash(key);
    for (i=0; i<dict->size; i++) {
        if (dict->key[i]==NULL)
            continue ;
        /* Compare hash */
        if (hash==dict->hash[i]) {
            /* Compare string, to avoid hash collisions */
            if (!strcmp(key, dict->key[i])) {
                /* Found key */
                break;
            }
        }
    }
    if (i>=dict->size)
        /* Key not found */
        return;

    free(dict->key[i]);
    dict->key[i] = NULL ;
    if (dict->val[i]!=NULL) {
        free(dict->val[i]);
        dict->val[i] = NULL;
    }
    dict->hash[i] = 0;
    dict->n --;
}

SMFList_T *smf_dict_get_keys(SMFDict_T *dict) {
    int i = 0;
    SMFList_T *l = NULL;
    assert(dict);

    if (smf_list_new(&l,_string_list_destroy)!=0) 
        return NULL;

    for (i=0 ; i<dict->size ; i++) {
        if (dict->key[i]) {
            if (smf_list_append(l, strdup(dict->key[i])) != 0) {
                smf_list_free(l);
                return NULL;
            }
        }
    }
    return l;
}

void smf_dict_map(SMFDict_T *dict, void(*func)(char *key,char *value, void *args), void *args) {
    assert(dict);
    SMFList_T *list = NULL;
    SMFListElem_T *elem = NULL;
    char *key = NULL;
    char *value = NULL;

    assert(dict);
    
    list = smf_dict_get_keys(dict);
    elem = cmime_list_head(list);
    while(elem != NULL) {
        key = (char *)smf_list_data(elem);
        value = smf_dict_get(dict,key);
        func(key,value,args);
        elem = elem->next;
    }

    smf_list_free(list);
}
