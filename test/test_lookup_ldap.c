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
#include "../src/smf_lookup_ldap.h"


int main (int argc, char const *argv[]) {
    char *ldap_uri = NULL;

    ldap_uri = ldap_get_uri("bla.host.de");

    printf("[%s]", ldap_uri);

    if(ldap_uri != NULL)
        free(ldap_uri);
    return(0);
}

