/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2010 Axel Steiner and SpaceNet AG
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
#include "../src/smf_lookup.h"
#include "../src/smf_lookup_private.h"
#include "../src/smf_settings_private.h"

int main (int argc, char const *argv[]) {
    char *ldap_uri = NULL;
    int scope_retval;
    int ldap_connect_retval;
    char *conn_type = "failover";
    char *pw = "test123";
    char *bind_dn = "dc=example,dc=com";

   	char **host = malloc(2*sizeof(char));
 			host[0] = strdup("10.211.55.61");
 			host[1] = strdup("10.211.55.61");

 			ldap_uri = ldap_get_uri("10.211.55.61", 389);
 			scope_retval = ldap_get_scope("subtree");

    SMFSettings_T *settings = smf_settings_new();
    smf_settings_set_ldap_host(settings, host);
 			smf_settings_set_backend_connection(settings, conn_type);
 			
 			smf_settings_set_ldap_bindpw(settings, pw);
 			smf_settings_set_ldap_binddn(settings, bind_dn);
				smf_lookup_ldap_connect(ldap_uri, settings);

 			printf("settings->ldap_bindpw: [%s]\n", settings->ldap_bindpw);
 			printf("settings->ldap_host: [%s]", settings->ldap_host[0]);

    if(ldap_uri != NULL)
        free(ldap_uri);

    if(host != NULL)
    				free(host);

    return(0);
}

