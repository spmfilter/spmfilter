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

#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_email_address.h"

#include "test.h"

int main (int argc, char const *argv[]) {
    SMFEmailAddress_T *ea = NULL;
    SMFEmailAddress_T *ea2 = NULL;
    char *s = NULL;

    g_printf("Start SMFEmailAddress_T tests...\n");

    g_printf("* testing smf_email_address_new()...\t\t\t\t");
    ea = smf_email_address_new();
    assert(ea);
    g_printf("passed\n");

    g_printf("* testing smf_email_address_parse_string()...\t\t\t");
    ea2 = smf_email_address_parse_string(test_addr);
    assert(ea2);
    g_printf("passed\n");   

    g_printf("* testing smf_email_address_to_string()...\t\t\t");
    s = smf_email_address_to_string(ea2);
    
    if (strcmp(test_addr,s) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    free(s);
    smf_email_address_free(ea2);

    g_printf("* testing smf_email_address_set_type()...\t\t\t");
    smf_email_address_set_type(ea,SMF_EMAIL_ADDRESS_TYPE_TO);
    g_printf("passed\n");

    g_printf("* testing smf_email_address_get_type()...\t\t\t");
    if (smf_email_address_get_type(ea) != SMF_EMAIL_ADDRESS_TYPE_TO) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    g_printf("* testing smf_email_address_set_name()...\t\t\t");
    smf_email_address_set_name(ea,test_name);
    g_printf("passed\n");

    g_printf("* testing smf_email_address_get_name()...\t\t\t");
    if (strcmp(test_name,smf_email_address_get_name(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    g_printf("* testing smf_email_address_set_email()...\t\t\t");
    smf_email_address_set_email(ea,test_email);
    g_printf("passed\n");

    g_printf("* testing smf_email_address_get_email()...\t\t\t");
    if (strcmp(test_email,smf_email_address_get_email(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
    
    g_printf("* testing smf_email_address_free()...\t\t\t\t");
    smf_email_address_free(ea);
    g_printf("passed\n");
    
    return 0;
}
