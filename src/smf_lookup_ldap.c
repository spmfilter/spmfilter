/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner, Werner Detter and SpaceNet AG
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,+
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#define _GNU_SOURCE

#include <lber.h>
#include <ldap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_internal.h"
#include "smf_core.h"
#include "smf_session.h"

#define THIS_MODULE "lookup_ldap"

char *smf_lookup_ldap_get_rand_host(SMFSettings_T *settings) {
    assert(settings);
    int random;
    SMFListElem_T *e = NULL;
    int count = 0;

    TRACE(TRACE_DEBUG,"trying to get random ldap server");
    srand(time(NULL));
    random = rand() % smf_list_size(settings->ldap_host);
    e = smf_list_head(settings->ldap_host);

    while(e != NULL) {     
        count++;
        if(count != random) {
            return (char *)smf_list_data(e);
        }
        e = e->next;
    }
    return NULL;
}

char *smf_lookup_ldap_get_uri(SMFSettings_T *settings, char *host) {
    char *uri = NULL;
    char *s = NULL;

    assert(settings);

    if (host != NULL) {
        asprintf(&uri,"ldap://%s:%d",host,settings->ldap_port);
        return uri;
    }

    if(settings->ldap_uri) {
        uri = strdup(settings->ldap_uri);
    } else {
        if (strcasecmp(settings->backend_connection,"balance") == 0) {
            s = smf_lookup_ldap_get_rand_host(settings);
        } else {
            SMFListElem_T *e = NULL;
            e = smf_list_head(settings->ldap_host);
            s = (char *)smf_list_data(e);
        }
        
        asprintf(&uri,"ldap://%s:%d",s,settings->ldap_port);
    }

    return uri;
}

void smf_lookup_ldap_init_ld(SMFSettings_T **settings,char *uri) {
    int version;
    LDAP *ld = NULL;
    
    if ((*settings)->lookup_connection != NULL)
        ldap_unbind_ext_s((LDAP *)(*settings)->lookup_connection,NULL,NULL);

    if (ldap_initialize(&ld, uri) != LDAP_SUCCESS) {
        TRACE(TRACE_ERR, "ldap_initialize() failed");
        (*settings)->lookup_connection = NULL;
        return;
    } else 
        (*settings)->lookup_connection = ld;

    version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    
    if ((*settings)->ldap_referrals == 1) {
        ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_ON);
        TRACE(TRACE_LOOKUP, "set ldap referrals to on");
    } else {
        ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_OFF);
        TRACE(TRACE_LOOKUP, "set ldap referrals to off");
    }
}

int smf_lookup_ldap_connect(SMFSettings_T *settings)  {  
    int ret = -1;
    char *uri = NULL;
    char *host = NULL;
    SMFListElem_T *elem;

    assert(settings);

    if(settings->backend_connection == NULL) {
        TRERR("settings->backend_connection is NULL, aborted");
        return ret;
    }

    uri = smf_lookup_ldap_get_uri(settings,NULL);
    smf_lookup_ldap_init_ld(&settings,uri);

    if ((ret = smf_lookup_ldap_bind(settings))!=0) {
        TRACE(TRACE_ERR,"ldap connection to [%s] failed\n",uri);
        /* check failover connections */
        elem = smf_list_head(settings->ldap_host);
        while(elem != NULL) {
            /* free failed connection stuff */
            if (uri != NULL) free(uri);

            /* build connection for next server */
            host = (char *)smf_list_data(elem);
            uri = smf_lookup_ldap_get_uri(settings, host);
            TRACE(TRACE_DEBUG,"trying new connection to [%s]\n",uri);
            smf_lookup_ldap_init_ld(&settings,uri);
            if ((ret = smf_lookup_ldap_bind(settings))==0) 
                break;
            elem = elem->next;
        }
    }

    if (ret == 0) {
        TRACE(TRACE_LOOKUP,"successfully bound to [%s]\n",uri);
    } else {
        TRACE(TRACE_LOOKUP,"failed to bind to [%s]\n",uri);
    }

    free(uri);

    return ret;
}

int smf_lookup_ldap_bind(SMFSettings_T *settings) {
    int err;
    struct berval *cred;
    LDAP *ld = (LDAP *)settings->lookup_connection;
    assert(ld);

    assert(settings);

    if(settings->ldap_base == NULL) {
        TRERR("settings->ldap_bindpw is NULL, aborted");
        return -1;
    }
    if(settings->ldap_binddn == NULL) {
        TRERR("settings->ldap_binddn is NULL, aborted");
        return -1;
    }

    cred = malloc(sizeof(struct berval));
    cred->bv_len = strlen(settings->ldap_bindpw);
    cred->bv_val = settings->ldap_bindpw;

    if ((err = ldap_sasl_bind_s(ld,settings->ldap_binddn,LDAP_SASL_SIMPLE,cred,NULL,NULL,NULL))) {
        TRACE(TRACE_ERR,"ldap bind failed: [%s]\n",ldap_err2string(err));
        free(cred);
        return -1;
    }

    free(cred);
    
    return 0;
}

void smf_lookup_ldap_disconnect(SMFSettings_T *settings) {
    LDAP *ld = NULL;
    assert(settings);
    
    ld = (LDAP *)settings->lookup_connection;
    if (ld != NULL) {
        ldap_unbind_ext_s(ld,NULL,NULL);
        TRACE(TRACE_LOOKUP, "successfully bound ldap connection");
        settings->lookup_connection = NULL;
    }
}


int smf_lookup_ldap_get_scope(SMFSettings_T *settings) {
    assert(settings);
    if (strcasecmp(settings->ldap_scope,"subtree") == 0)
        return LDAP_SCOPE_SUBTREE;
    else if (strcasecmp(settings->ldap_scope,"onelevel") == 0)
        return LDAP_SCOPE_ONELEVEL;
    else if (strcasecmp(settings->ldap_scope,"base") == 0)
        return LDAP_SCOPE_BASE;
    else
        return LDAP_SCOPE_SUBTREE;
}

LDAP *smf_lookup_ldap_get_connection(SMFSettings_T *settings) {
    assert(settings);
    LDAP *ld = NULL;
    ld = (LDAP *)settings->lookup_connection;

    if (ld == NULL) {
        if (smf_lookup_ldap_connect(settings) != 0) {
            TRACE(TRACE_LOOKUP,"failed to get ldap connection");
            return NULL;
        }
        ld = (LDAP *)settings->lookup_connection;
    }

    return ld;
}


SMFList_T *smf_lookup_ldap_query(SMFSettings_T *settings, const char *q, ...) {
    va_list ap, cp;
    int i,value_count;

    LDAP *c = NULL;
    LDAPMessage *msg = NULL;
    LDAPMessage *entry = NULL;
  
    char *query;
    struct berval **bvals;
    BerElement *ptr;
    SMFList_T *result;
    
    assert(q);
    assert(settings);

    if(settings->ldap_base == NULL) {
        TRERR("settings->ldap_base is NULL, aborted");
        return NULL;
    }

    c = smf_lookup_ldap_get_connection(settings);
    if (c == NULL) {
        TRACE(TRACE_ERR,"no active connection availbable");
        return NULL;
    }


    if (smf_list_new(&result,smf_internal_dict_list_destroy)!=0) {
        return NULL;
    } else {
        va_start(ap, q);
        va_copy(cp, ap);
        query = (char *)malloc(strlen(q) + 1);
        vsprintf(query,q,cp);
        va_end(cp);
        smf_core_strstrip(query);

        if (strlen(query) == 0)
            return NULL;

        TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
        TRACE(TRACE_DEBUG,"[%p] [%s]",c,query);

        if (ldap_search_ext_s(c,settings->ldap_base,smf_lookup_ldap_get_scope(settings),query,NULL,0,NULL, NULL, NULL, 0, &msg) != LDAP_SUCCESS)
            TRACE(TRACE_ERR,"[%p] query [%s] failed",c, query);

        if(ldap_count_entries(c,msg) <= 0) {
            TRACE(TRACE_LOOKUP,"[%p] nothing found",c);
            free(query);
            return NULL;
        } else {
            TRACE(TRACE_LOOKUP,"[%p] found [%d] entries", c, ldap_count_entries(c,msg));
        }

        for (entry = ldap_first_entry(c, msg); entry != NULL; entry = ldap_next_entry(c,entry)) {
            char *attr = NULL;
            SMFDict_T *d = smf_dict_new();

            for(attr = ldap_first_attribute(c, msg, &ptr); attr != NULL; attr = ldap_next_attribute(c, msg, ptr)) {
                char *data = NULL;
                
                bvals = ldap_get_values_len(c, entry, attr);
                value_count = ldap_count_values_len(bvals);
                TRACE(TRACE_LOOKUP,"found attribute [%s] in entry [%p] with [%d] values", attr, entry, value_count);
                
                data = (char *)calloc(1,sizeof(char));
                
                for (i = 0; i < value_count; i++) {
                    if(i == 0) {
                        smf_core_strcat_printf(&data, "%s", (char *)((struct berval)*bvals[i]).bv_val);
                    } else {    
                        smf_core_strcat_printf(&data, ",%s", (char *)((struct berval)*bvals[i]).bv_val);
                    }
                }
            
                smf_dict_set(d,attr,data);
                ldap_memfree(attr);
                free(data);

                
                ldap_value_free_len(bvals);
            }

            if (smf_list_append(result,d) != 0)
                return NULL;

        }
    }  
    ber_free(ptr,0);
    ldap_msgfree(msg);
    ldap_msgfree(entry);
    free(query);
    return result;   
}

