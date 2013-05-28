/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2013 Axel Steiner and SpaceNet AG
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

#define THIS_MODULE "internal"
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/times.h>

#include "smf_internal.h"
#include "smf_trace.h"
#include "smf_dict.h"
#include "smf_session.h"
#include "smf_lookup.h"

void smf_internal_string_list_destroy(void *data) {
    char *s = (char *)data;
    assert(data);
    free(s);
}

void smf_internal_dict_list_destroy(void *data) {
    assert(data);
    smf_dict_free((SMFDict_T *)data);
}
   
void smf_internal_user_data_list_destroy(void *data) {
    assert(data);
    SMFUserData_T *user_data = (SMFUserData_T *)data;
    if (user_data->email != NULL) 
        free(user_data->email);

    smf_dict_free(user_data->data);
    free(data);
}

int smf_internal_user_match(SMFSession_T *session, SMFList_T *result_attributes, SMFDict_T *d, char *addr) {
    SMFListElem_T *key_elem = NULL;
    SMFListElem_T *attr_elem = NULL;
    SMFList_T *keys = smf_dict_get_keys(d);
    char *attr = NULL;
    char *lookup_attr = NULL;
    char *value = NULL;

    key_elem = smf_list_head(keys);
    while(key_elem != NULL) {
        attr = (char *)smf_list_data(key_elem);
        attr_elem = smf_list_head(result_attributes);
        while(attr_elem != NULL) {
            lookup_attr = (char *)smf_list_data(attr_elem);
            if (strcmp(lookup_attr,attr)==0) {
                value = smf_dict_get(d, attr);
                if (strcmp(value,addr)==0) {
                    STRACE(TRACE_DEBUG,session->id,"found matching entry for address [%s] within attribute [%s]",addr,attr);
                    smf_list_free(keys);
                    return 1;
                }
            }
            attr_elem = attr_elem->next;
        }

        key_elem = key_elem->next;
    }

    smf_list_free(keys);
    return 0;
}

SMFDict_T *smf_internal_copy_user_data(SMFDict_T *origin) {
    SMFDict_T *d = smf_dict_new();
    SMFList_T *keys = smf_dict_get_keys(origin); 
    SMFListElem_T *e = NULL;
    char *k = NULL;
    char *v = NULL;

    e = smf_list_head(keys);
    while(e != NULL) {
        k = (char *)smf_list_data(e);
        v = smf_dict_get(origin,k);
        smf_dict_set(d,k,v);
        e = e->next;
    }

    smf_list_free(keys);
    return d;
}

SMFDict_T *smf_internal_get_user_result(SMFSettings_T *settings, SMFSession_T *session, SMFList_T *result, char *addr) {
    SMFDict_T *d = NULL;
    SMFListElem_T *e = NULL;

    e = smf_list_head(result);

#ifdef HAVE_LDAP
    while(e != NULL) {
        d = (SMFDict_T *)smf_list_data(e);
        if (smf_internal_user_match(session,settings->ldap_result_attributes,d,addr)==1)
            break;
        
        e = e->next;
    }
#elif defined HAVE_SQL
    if (e != NULL)
        d = (SMFDict_T *)smf_list_data(e);
#endif

    return smf_internal_copy_user_data(d);
}

int smf_internal_query_user(SMFSettings_T *settings, SMFSession_T *session, char *addr) {
    char *query = NULL;
    SMFList_T *result = NULL;
    SMFUserData_T *user_data = NULL;
    
#ifdef HAVE_LDAP
    if (smf_core_expand_string(settings->ldap_user_query,addr,&query) == -1) {
        STRACE(TRACE_ERR, session->id, "failed to expand user query");
        return -1;
    }

    result = smf_lookup_ldap_query(settings, session, query);
#elif defined HAVE_SQL
    if (smf_core_expand_string(settings->sql_user_query,addr,&query) == -1) {
        STRACE(TRACE_ERR, session->id, "failed to expand user query");
        return -1;
    }

    result = smf_lookup_sql_query(settings, session, query);
#endif

    if (result != NULL) {
        user_data = (SMFUserData_T *)calloc((size_t)1, sizeof(SMFUserData_T));
        user_data->email = strdup(addr);

        user_data->data = smf_internal_get_user_result(settings, session, result, addr);
        smf_list_append(session->local_users, (void *)user_data);
        smf_list_free(result);
    } 

    if (query != NULL) {
        free(query);
        query = NULL;
    }

    return 0;
}

char *smf_internal_build_module_path(const char *libdir, const char *modname) {
    char *path = NULL;
    char *t = NULL;

    if (strncmp(modname,"lib",3)==0) {
#ifdef __APPLE__
        asprintf(&t,"%s.dylib",modname);
#else
        t = strdup(modname);
#endif
    } else {
#ifdef __APPLE__
        asprintf(&t,"lib%s.dylib", modname);
#else
        asprintf(&t,"lib%s.so",modname);
#endif
    }
    asprintf(&path,"%s/%s",libdir,t);
    free(t);

    return path;
}

char *smf_internal_strip_email_addr(char *addr) {
    char *p1 = NULL;
    char *p2 = NULL;
    char *out = NULL;
    size_t len1, len2, offset;  

    if (*addr == '<') 
        p1 = addr;
    else
        p1 = strchr(addr,'<');
    
    if (p1 != NULL) {
        len1 = strlen(++p1);
        if ((p2 = strchr(p1,'>')) != NULL) {
            len2 = strlen(p2);
            offset = len1 - len2;
            out = (char *)calloc(offset + 1, sizeof(char));
            strncpy(out,p1,offset);
            out[strlen(out)] = '\0';
        } else 
            out = strdup(addr);
    } else
        out = strdup(addr);
    
    return out;
}

ssize_t smf_internal_readn(int fd, void *buf, size_t nbyte) {
    size_t n;
    ssize_t br;
    char *p = buf;

    for (n = nbyte; n > 0; n -= br, p += br) {
        if ((br = read(fd,p,n)) < 0) {
            if (errno == EINTR)
                br = 0;
            else
                return -1;
        } else if (br == 0)
            return (nbyte - n);
    }

    return nbyte;
}

ssize_t smf_internal_writen(int fd, const void *buf, size_t nbyte) {
    size_t n;
    ssize_t bw;
    const char *p = buf;

    for (n = nbyte; n > 0; n -= bw, p += bw) {
        if ((bw = write(fd,p,n)) < 0) {
            if (errno == EINTR)
                bw = 0;
            else
                return -1;
        } else if (bw == 0)
            return (nbyte - 1);
    }

    return nbyte;
}

ssize_t smf_internal_readline(int fd, void *buf, size_t nbyte, void **help) {
    size_t n;
    ssize_t br;
    char c, *ptr = buf;
    readline_t *rl = *help;

    if (rl == NULL) {
        if ((rl = malloc(sizeof(readline_t))) == NULL)
            return -1;

        rl->count = 0;
        rl->current = rl->buf; 
        *help = rl;
    }   

    for (n = 1; n < nbyte; n++) {
        if ((br = smf_internal_readcbuf(fd,&c,rl)) < 0)
            return -1;

        *ptr++ = c;
        if ((br == 0) || ( c == '\n'))
            break;
    }

    if ((br == 0) && (n == 1))
        return 0;

    *ptr = 0;
    return n;
}

ssize_t smf_internal_readcbuf(int fd, char *buf, readline_t *rl) {
    while(rl->count < 1) {
        if ((rl->count = read(fd, rl->buf, sizeof(rl->buf))) < 0) {
            if (errno == EINTR)
                rl->count = 0;
            else
                return -1;
        } else if (rl->count == 0)
            return 0;

        rl->current = rl->buf;
    }

    *buf = *rl->current++;
    rl->count--;

    return 1;
}

struct tms smf_internal_init_runtime_stats(void) {
    struct tms start_acct;
    times(&start_acct);
    
    return start_acct;
}

void smf_internal_print_runtime_stats(struct tms start_acct, const char *sid) {
    struct tms end_acct;

    times(&end_acct);
    STRACE(TRACE_DEBUG,sid,"CPU time (user and system): %0.5f",
        (float)(end_acct.tms_utime - start_acct.tms_utime)  + /* User CPU time */
        (float)(end_acct.tms_stime - start_acct.tms_stime) + /* System CPU time */
        (float)(end_acct.tms_cutime - start_acct.tms_cutime) + /* User CPU time of terminated child processes */
        (float)(end_acct.tms_cstime - start_acct.tms_cstime) /* System CPU time of terminated child processes */
    );

    return;
}

char *smf_internal_determine_linebreak(const char *s) {
    assert(s);

    if (strstr(s,CRLF)!=NULL)
        return(CRLF);
    else if(strstr(s,LF)!=NULL)
        return(LF);
    else if(strstr(s,CR)!=NULL) 
        return(CR);
    else
        return(NULL);
}

int smf_internal_fetch_user_data(SMFSettings_T *settings, SMFSession_T *session) {
    SMFListElem_T *e1 = NULL;
    char *addr = NULL;
    
    if (settings->backend == NULL) 
        return 0;

    if ((strcmp(settings->backend,"ldap")==0) && (settings->ldap_user_query==NULL)) {
        STRACE(TRACE_WARNING, session->id, "no user_query defined for ldap backend");
        return 0; 
    }

    if ((strcmp(settings->backend,"sql")==0) && (settings->sql_user_query==NULL)) {
        STRACE(TRACE_WARNING, session->id, "no user_query defined for sql backend");
        return 0; 
    }

    e1 = smf_list_head(session->envelope->recipients);
    while (e1 != NULL) {
        addr = (char *)smf_list_data(e1);
        STRACE(TRACE_DEBUG, session->id, "fetching user data for [%s]", addr);

        if(smf_internal_query_user(settings,session,addr)!=0) {
            STRACE(TRACE_ERR, session->id, "failed to fetch user data for [%s]", addr);
        }
        e1 = e1->next;
    }

    if (session->envelope->sender != NULL) {
        STRACE(TRACE_DEBUG, session->id, "fetching user data for [%s]", session->envelope->sender);
        if(smf_internal_query_user(settings,session,session->envelope->sender)!=0) {
            STRACE(TRACE_ERR, session->id, "failed to fetch user data for [%s]", session->envelope->sender);
        }
    }

    return 0;
}
