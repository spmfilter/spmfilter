/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Werner Detter and SpaceNet AG
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <db.h>
#include <glib/gprintf.h>
#include <assert.h>
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings_private.h"


/*
 *
 * this tests can only run successfull if there is a local ldap server available with the following
 * scheme installed:
 * 

 // base.ldif
 dn: dc=example,dc=com
 dc: example
 objectClass: top
 objectClass: domain

 dn: ou=People,dc=example,dc=com
 ou: People
 objectClass: top
 objectClass: organizationalUnit

 dn: ou=Group,dc=example,dc=com
 ou: Group
 objectClass: top
 objectClass: organizationalUnit

 //user2.ldif
 dn: uid=test2,ou=People,dc=example,dc=com
 uid: test2
 cn: Test User
 objectClass: account
 objectClass: posixAccount
 objectClass: top
 userPassword: {SSHA}DHiQoi+pqOQXFP28g+NIyQmagm1xxjNr
 loginShell: /bin/bash
 uidNumber: 500
 gidNumber: 500
 homeDirectory: /home/test2

 * ldapadd -x -D "cn=Manager,dc=example,dc=com" -f /etc/openldap/base.ldif -W
 * ldapadd -x -D "cn=Manager,dc=example,dc=com" -f /etc/openldap/user2.ldif -W
 */

#define LDAP_HOST_1 "127.0.0.1"
#define LDAP_HOST_2 "127.0.0.1"
#define LDAP_PORT 389
#define LDAP_BIND_DN "uid=test2,ou=People,dc=example,dc=com"
#define LDAP_PSW "test"
#define LDAP_BASE "uid=test2,ou=People,dc=example,dc=com"
#define LDAP_SCOPE "subtree"
#define LDAP_QUERY_STRING "(uid=test2)"
#define LDAP_QUERY_STRING_RESULT "500"
#define LDAP_CONN_TYPE "failover"


int main (int argc, char const *argv[]) {

    int scope_retval;
    int ldap_connect_retval;
    char *conn_type = LDAP_CONN_TYPE;
    char *pw = LDAP_PSW;
    char *bind_dn = LDAP_BIND_DN;
    SMFLookupResult_T *ldapresult = NULL;
    SMFLdapValue_T *uidNumber = NULL;
    int j;

    char **host = (char**)g_malloc(3*sizeof(char*));
    host[0] = g_strdup(LDAP_HOST_1);
    host[1] = g_strdup(LDAP_HOST_1);
    host[2] = '\0';

    SMFSettings_T *settings = smf_settings_new();
    smf_settings_set_ldap_host(settings, host);
    smf_settings_set_ldap_scope(settings, "subtree");
    smf_settings_set_backend_connection(settings, conn_type); 
    smf_settings_set_ldap_bindpw(settings, pw); 
    smf_settings_set_ldap_binddn(settings, bind_dn);
    smf_settings_set_ldap_base(settings, LDAP_BASE);
    smf_settings_set_ldap_referrals(settings, 0);  

    printf("settings->backend_connection: [%s]\n", settings->backend_connection);
    smf_lookup_ldap_connect(settings);
    printf("settings->ldap_uri: [%s]\n", settings->ldap_uri);

    
    ldapresult = smf_lookup_ldap_query(settings, LDAP_QUERY_STRING);
    
    if(ldapresult != NULL) {
        if(ldapresult->len > 0) {
            for(j=0; j < ldapresult->len; j++) {
             SMFLookupElement_T *elem = smf_lookup_result_index(ldapresult,j);
             uidNumber = (SMFLdapValue_T *) smf_lookup_element_get(elem, "uidNumber");
             printf("RESULT:[%s]\n", uidNumber->data[0]);
             assert(strcmp(uidNumber->data[0],LDAP_QUERY_STRING_RESULT)==0);
            }
        }
    }
    
    smf_lookup_result_free(ldapresult);
    smf_lookup_ldap_disconnect(settings);
    smf_settings_free(settings);

    if(host[0] != NULL)
        free(host[0]);

     if(host[1] != NULL)
        free(host[1]);

     if(host != NULL)
        g_free(host);


    return 0;
}

