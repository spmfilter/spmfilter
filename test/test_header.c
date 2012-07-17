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

#include <stdio.h>
#include <assert.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_header.h"

#define HEADER_KEY "X-Foo"
#define HEADER_VALUE "foobar"
#define HEADER_STRING "X-Foo: foobar"

int main(int argc, char const *argv[]) {
    SMFHeader_T *header = NULL;
    char *s = NULL;
    int i;
    g_printf("Start SMFHeader_T tests...\n");

    g_printf("* testing smf_header_new()...\t\t\t\t");
    header = smf_header_new();
    assert(header);
    g_printf("passed\n");


    g_printf("* testing smf_header_set_name()...\t\t\t");
    smf_header_set_name(header,HEADER_KEY);
    g_printf("passed\n");

    g_printf("* testing smf_header_get_name()...\t\t\t");
    s = smf_header_get_name(header);
    if (strcmp(s,HEADER_KEY)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");


    g_printf("* testing smf_header_set_value()...\t\t\t");
    smf_header_set_value(header,HEADER_VALUE,0);
    g_printf("passed\n");

    g_printf("* testing smf_header_get_value()...\t\t\t");
    s = smf_header_get_value(header,0);
    if (strcmp(s,HEADER_VALUE)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_header_get_count()...\t\t\t");
    i = smf_header_get_count(header);
    if (i!=1) {
        g_printf("failed\n");
        return -1;
    } 
    g_printf("passed\n");

    g_printf("* testing smf_header_to_string()...\t\t\t");
    s = smf_header_to_string(header);
    if (strcmp(s,HEADER_STRING)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);
    
    g_printf("* testing smf_header_free()...\t\t\t\t");
    smf_header_free(header);
    g_printf("passed\n");
    return 0;
}