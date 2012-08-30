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
#include <db.h>
#include <assert.h>
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings_private.h"


/*
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

#define LDAP_HOST_1 "hostwhichdoesntexist"
#define LDAP_HOST_2 "localhost"
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

    char *host1 = strdup(LDAP_HOST_1);
    char *host2 = strdup(LDAP_HOST_2);

    SMFList_T *result = NULL;
    SMFListElem_T *e = NULL;
    SMFSettings_T *settings = smf_settings_new();
    SMFDict_T *d = NULL;

    smf_settings_add_ldap_host(settings, host1);
    smf_settings_add_ldap_host(settings, host2);
    smf_settings_set_ldap_scope(settings, "subtree");
    smf_settings_set_backend_connection(settings, conn_type); 
    smf_settings_set_ldap_bindpw(settings, pw); 
    smf_settings_set_ldap_binddn(settings, bind_dn);
    smf_settings_set_ldap_base(settings, LDAP_BASE);
    smf_settings_set_ldap_referrals(settings, 0);
   
    if(smf_lookup_ldap_connect(settings) == 0) {
    
        result = smf_lookup_ldap_query(settings, LDAP_QUERY_STRING);    
        e = smf_list_head(result);

        while(e != NULL) {
            d = (SMFDict_T *)smf_list_data(e);
            //printf("[%s]",smf_dict_get(d,"uidNumber"));
            assert(strcmp(smf_dict_get(d,"uidNumber"),LDAP_QUERY_STRING_RESULT)==0);
            e = e->next;
        }

        smf_list_free(result);
        smf_lookup_ldap_disconnect(settings);
    } else {
        printf("unable to establish ldap connection");
    }
   
    smf_settings_free(settings);
    
    return 0;
}

