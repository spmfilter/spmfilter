/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner, Werner Detter and SpaceNet AG
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
#include <lber.h>
#include <ldap.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <assert.h>

#include "smf_trace.h"
#include "smf_settings.h"
#include "smf_lookup.h"
#include "smf_lookup_private.h"
#include "smf_core.h"
#include "smf_session.h"
#include "smf_dict.h"
#include "smf_list.h"

#define THIS_MODULE "ldap_lookup"

void _ldap_result_list_destroy(void *data) {
    assert(data);
    //FIXME
    //smf_dict_free((SMFDict_T *)data);
}



int ldap_get_scope(SMFSettings_T *settings) {
    assert(settings);
    if (g_ascii_strcasecmp(settings->ldap_scope,"subtree") == 0)
        return LDAP_SCOPE_SUBTREE;
    else if (g_ascii_strcasecmp(settings->ldap_scope,"onelevel") == 0)
        return LDAP_SCOPE_ONELEVEL;
    else if (g_ascii_strcasecmp(settings->ldap_scope,"base") == 0)
        return LDAP_SCOPE_BASE;
    else
        return LDAP_SCOPE_SUBTREE;
}



char *ldap_get_uri(char *ldap_host, int ldap_port) {
    char *uri;
    assert(ldap_host);
    assert(ldap_port);
    uri = g_strdup_printf("ldap://%s:%d",ldap_host,ldap_port);
    return uri;
}



int smf_lookup_ldap_connect(SMFSettings_T *settings)  {  
    int ret;
    int version;
    char *uri = NULL;
    char *host = NULL;
    LDAP *ld = NULL;

    assert(settings);


    if(settings->backend_connection == NULL) {
        TRERR("settings->backend_connection is NULL, aborted");
        return -1;
    }

    if(settings->ldap_uri) {
        if ((ret = ldap_initialize(&ld, settings->ldap_uri) != LDAP_SUCCESS)) 
             TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
     } else {
        if (g_ascii_strcasecmp(settings->backend_connection,"balance") == 0) {
            host = ldap_get_rand_host(settings);
        } else {
            SMFListElem_T *e = NULL;
            e = smf_list_head(settings->ldap_host);
            host = strdup((char *)smf_list_data(e));
            smf_settings_set_active_lookup_host(settings, (char *)smf_list_data(e));
        }
        uri = ldap_get_uri(host,389);
        settings->ldap_uri = strdup(uri);

        if ((ret = ldap_initialize(&ld, settings->ldap_uri) != LDAP_SUCCESS)) { 
             TRACE(TRACE_ERR, "ldap_initialize() returned [%d]", ret);
        }
     }    

    version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);
    
    if (settings->ldap_referrals == 1) {
        ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_ON);
        TRACE(TRACE_LOOKUP, "set ldap referrals to on");
    } else {
        ldap_set_option(ld, LDAP_OPT_REFERRALS, (void *)LDAP_OPT_OFF);
        TRACE(TRACE_LOOKUP, "set ldap referrals to off");
    }

    settings->ldap_connection = ld;

    if (smf_ldap_bind(settings) != 0) { 
        if (ldap_failover_connect(settings) != 0) {
            TRERR("failover connection failed");
            return -1;
        }
    }

    if(uri != NULL) {
        g_free(uri);
    }

    return 0;

}


char *ldap_get_rand_host(SMFSettings_T *settings) {
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



int smf_ldap_bind(SMFSettings_T *settings) {
    assert(settings);
    
    int err;
    struct berval *cred;
    LDAP *ld = (LDAP *)settings->ldap_connection;

    if(settings->ldap_base == NULL) {
        TRERR("settings->ldap_bindpw is NULL, aborted");
        return -1;
    }
    if(settings->ldap_binddn == NULL) {
        TRERR("settings->ldap_binddn is NULL, aborted");
        return -1;
    }
    if(settings->ldap_uri == NULL) {
        TRERR("settings->ldap_uri is NULL, aborted");
        return -1;
    }

    cred = malloc(sizeof(struct berval));
    cred->bv_len = strlen(settings->ldap_bindpw);
    cred->bv_val = settings->ldap_bindpw;

    if ((err = ldap_sasl_bind_s(ld,settings->ldap_binddn,LDAP_SASL_SIMPLE,cred,NULL,NULL,NULL))) {
        TRACE(TRACE_ERR, "ldap_sasl_bind_s on host [%s] failed: %s", settings->ldap_uri, ldap_err2string(err));
        free(cred);
        return -1;
    }

    TRACE(TRACE_LOOKUP, "successfully bound to host [%s]", settings->ldap_uri);
    free(cred);
    
    return 0;
    
}




int ldap_failover_connect(SMFSettings_T *settings) {

    assert(settings);
    SMFList_T *l = NULL;
    SMFListElem_T *e = NULL;
    char *uri=NULL;

    l = smf_settings_get_ldap_hosts(settings);
    e = smf_list_head(l);

    while(e != NULL) {
        if(strcasecmp(smf_settings_get_active_lookup_host(settings), 
        (char *)smf_list_data(e)) != 0) {
            uri = ldap_get_uri((char *)smf_list_data(e), 389);
            smf_settings_set_active_lookup_host(settings, (char *)smf_list_data(e));
            settings->ldap_uri = g_strdup(uri);

            if (smf_ldap_bind(settings) != 0) {
                continue;
            } else {
                if(uri != NULL) {
                    free(uri);
                    return 0;
                }
            }
        }
        e = e->next;
    }

    if(uri != NULL)
        free(uri);

    return -1;
}

/*!
 * @fn LDAP *ldap_con_get(char *ldap_uri, SMFSettings_T *settings)
 * @brief get active LDAP connection, if no connection is available reconnect to LDAP server
 * @param ldap_uri Pointer to ldap_uri
 * @param settings Pointer to SMFSettings_T 
 * @returns Pointer to LDAP connection
 */
LDAP *ldap_con_get(SMFSettings_T *settings) {
    assert(settings);
    LDAP *ld = (LDAP *)settings->ldap_connection;
    if (!ld) {
        smf_lookup_ldap_connect(settings);
    }
    return ld;
}


void smf_lookup_ldap_disconnect(SMFSettings_T *settings) {
    assert(settings);
    
    LDAP *c = ldap_con_get(settings);
    if (c != NULL) {
        ldap_unbind_ext_s(c,NULL,NULL);
        TRACE(TRACE_LOOKUP, "unbind ldap server");
    }
}



SMFList_T *smf_lookup_ldap_query(SMFSettings_T *settings, const char *q, ...) {
    va_list ap, cp;
    int i,value_count;

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
        return -1;
    }

    LDAP *c = ldap_con_get(settings);

    if (smf_list_new(&result,_ldap_result_list_destroy)!=0) {
        return NULL;
    } else {
        va_start(ap, q);
        va_copy(cp, ap);
        query = g_strdup_vprintf(q, cp);
        va_end(cp);
        g_strstrip(query);

        if (strlen(query) == 0)
            return NULL;

        TRACE(TRACE_LOOKUP,"[%p] [%s]",c,query);
        TRACE(TRACE_DEBUG,"[%p] [%s]",c,query);

        if (ldap_search_ext_s(c,settings->ldap_base,ldap_get_scope(settings),query,NULL,0,NULL, NULL, NULL, 0, &msg) != LDAP_SUCCESS)
            TRACE(TRACE_ERR,"[%p] query [%s] failed",c, query);

        if(ldap_count_entries(c,msg) <= 0) {
            TRACE(TRACE_LOOKUP,"[%p] nothing found",c);
            g_free(query);
            return NULL;
        } else {
            TRACE(TRACE_LOOKUP,"[%p] found [%d] entries", c, ldap_count_entries(c,msg));
        }

        for (entry = ldap_first_entry(c, msg); entry != NULL; entry = ldap_next_entry(c,entry)) {
            char *attr;
            SMFDict_T *d = smf_dict_new();
            for(attr = ldap_first_attribute(c, msg, &ptr); attr != NULL; attr = ldap_next_attribute(c, msg, ptr)) {
                char *data;
                bvals = ldap_get_values_len(c, entry, attr);
                value_count = ldap_count_values_len(bvals);
                TRACE(TRACE_LOOKUP,"found attribute [%s] in entry [%p] with [%d] values", attr, entry, value_count);
                //printf("found attribute [%s] in entry [%p] with [%d] values\n", attr, entry, value_count);

                data = (char *)calloc(1,sizeof(char));
                for (i = 0; i < value_count; i++) {
                    if(i == 0) {
                        smf_core_strcat_printf(&data, "%s", (char *)((struct berval)*bvals[i]).bv_val);
                    } else {
                        smf_core_strcat_printf(&data, ",%s", (char *)((struct berval)*bvals[i]).bv_val);
                    }
                }
               
                //printf("attr: [%s], data: [%s]\n", attr, data);
                smf_dict_set(d,attr,data);
                free(attr);
                free(data);

                if (smf_list_append(result,d) != 0)
                    return NULL;

                ldap_value_free_len(bvals);
            }
        }
    }  
    ber_free(ptr,0);
    ldap_msgfree(msg);
    ldap_msgfree(entry);
    //g_free(query);
    return result;   
}


/*
void smf_lookup_ldap_check_user(char *ldap_uri, SMFEmailAddress_T *user, SMFSettings_T *settings) {
    char *query;
    smf_core_expand_string(settings->ldap_user_query,user->addr,&query);
    user->lr = NULL;
    user->lr = smf_lookup_ldap_query(ldap_uri, query, settings);
    free(query);
}
*/
