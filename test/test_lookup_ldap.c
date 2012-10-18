/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Werner Detter and SpaceNet AG
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

#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_list.h"
#include "../src/smf_internal.h"

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
 dn: uid=test,ou=People,dc=example,dc=com
 uid: test
 cn: Test User
 objectClass: account
 objectClass: posixAccount
 objectClass: top
 userPassword: {SSHA}DHiQoi+pqOQXFP28g+NIyQmagm1xxjNr
 loginShell: /bin/bash
 uidNumber: 500
 gidNumber: 500
 homeDirectory: /home/test

 * ldapadd -x -D "cn=Manager,dc=example,dc=com" -f /etc/openldap/base.ldif -W
 * ldapadd -x -D "cn=Manager,dc=example,dc=com" -f /etc/openldap/user2.ldif -W
 */

#define LDAP_HOST_1 "localhost"
#define LDAP_HOST_2 "notexisting.example.com"
#define LDAP_PORT 389
#define LDAP_BIND_DN "uid=test,ou=People,dc=example,dc=com"
#define LDAP_PSW "test"
#define LDAP_BASE "uid=test,ou=People,dc=example,dc=com"
#define LDAP_SCOPE "subtree"
#define LDAP_QUERY_STRING "(uid=test)"
#define LDAP_QUERY_STRING_RESULT "500"
#define LDAP_CONN_TYPE "failover"


int main (int argc, char const *argv[]) {
    char *conn_type = LDAP_CONN_TYPE;
    char *pw = LDAP_PSW;
    char *bind_dn = LDAP_BIND_DN;
    char *host1 = strdup(LDAP_HOST_1);
    char *host2 = strdup(LDAP_HOST_2);

    SMFList_T *result = NULL;
    SMFListElem_T *e = NULL;
    SMFSettings_T *settings = smf_settings_new();
    SMFDict_T *d = NULL;

    printf("Start smf_lookup_ldap tests...\n");
    printf("==================================================\n");
    printf("This tests can only run successfull if there\n\
is a local ldap server available with\n\
the following scheme installed:\n\n");
    printf("dn: dc=example,dc=com\n\
dc: example\n\
objectClass: top\n\
objectClass: domain\n\
\n\
dn: ou=People,dc=example,dc=com\n\
ou: People\n\
objectClass: top\n\
objectClass: organizationalUnit\n\
\n\
dn: ou=Group,dc=example,dc=com\n\
ou: Group\n\
objectClass: top\n\
objectClass: organizationalUnit\n\
\n\
dn: uid=test,ou=People,dc=example,dc=com\n\
uid: test\n\
cn: Test User\n\
objectClass: account\n\
objectClass: posixAccount\n\
objectClass: top\n\
userPassword: {SSHA}DHiQoi+pqOQXFP28g+NIyQmagm1xxjNr\n\
loginShell: /bin/bash\n\
uidNumber: 500\n\
gidNumber: 500\n\
homeDirectory: /home/test\n");
    printf("==================================================\n");

    smf_settings_add_ldap_host(settings, host1);
    smf_settings_set_ldap_port(settings, 389);
    smf_settings_set_ldap_scope(settings, "subtree");
    smf_settings_set_backend_connection(settings, conn_type); 
    smf_settings_set_ldap_bindpw(settings, pw); 
    smf_settings_set_ldap_binddn(settings, bind_dn);
    smf_settings_set_ldap_base(settings, LDAP_BASE);
    smf_settings_set_ldap_referrals(settings, 0);
   
    printf("* testing smf_lookup_ldap_connect()...\t\t\t\t");
    if(smf_lookup_ldap_connect(settings) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_lookup_ldap_query()...\t\t\t\t");
    result = smf_lookup_ldap_query(settings, LDAP_QUERY_STRING);    
    e = smf_list_head(result);
    d = (SMFDict_T *)smf_list_data(e);
    if (strcmp(smf_dict_get(d,"uidNumber"),"500")!=0) {
        printf("failed\n");
        return -1;
    }

    smf_list_free(result);
    printf("passed\n");

    printf("* testing smf_lookup_ldap_disconnect()...\t\t\t");
    smf_lookup_ldap_disconnect(settings);
    printf("passed\n");

    printf("* testing smf_lookup_ldap_connect() failover...\t\t\t");
    smf_list_free(settings->ldap_host);
    smf_list_new(&settings->ldap_host, smf_internal_string_list_destroy);
    host1 = strdup(LDAP_HOST_1);
    smf_settings_add_ldap_host(settings, host2);
    smf_settings_add_ldap_host(settings, host1);

    if(smf_lookup_ldap_connect(settings) != 0) {
        printf("failed\n");
        return -1;
    }
    smf_lookup_ldap_disconnect(settings);
    printf("passed\n");


    smf_settings_free(settings);
    
    return 0;
}

