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

#include "../src/smf_message.h"
#include "../src/smf_header.h"

#define TEST_NAME "John Doe"
#define TEST_EMAIL "foo@bar.com"
#define TEST_SENDER "John Doe <foo@bar.com>"
#define TEST_HEADER_NAME "X-Foo"
#define TEST_HEADER "X-Foo: foobar"

int main (int argc, char const *argv[]) {
    SMFMessage_T *msg = NULL;
    SMFEmailAddress_T *ea = NULL;
    SMFHeader_T *h = NULL;
    char *s = NULL;

    g_printf("Start SMFMessage_T tests...\n");

    g_printf("* testing smf_message_new()...\t\t\t\t");
    msg = smf_message_new();
    assert(msg);
    g_printf("passed\n");
  
    g_printf("* testing smf_message_set_sender()...\t\t\t");
    smf_message_set_sender(msg,TEST_SENDER);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_sender_string()...\t\t");
    s = smf_message_get_sender_string(msg);
    if (strcmp(s,TEST_SENDER) !=0 ) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
    free(s);  

    g_printf("* testing smf_message_get_sender()...\t\t\t");
    ea = smf_message_get_sender(msg);
    if (strcmp("John Doe ",smf_email_address_get_name(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    }

    if (strcmp("<foo@bar.com>",smf_email_address_get_email(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    } 
    g_printf("passed\n");

    g_printf("* testing smf_message_generate_message_id()...\t\t");
    s = smf_message_generate_message_id();
    assert(s);
    g_printf("passed\n");

    g_printf("* testing smf_message_set_message_id()...\t\t");
    smf_message_set_message_id(msg,s);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_message_id()...\t\t");
    if (strcmp(s,smf_message_get_message_id(msg)) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);

    g_printf("* testing smf_message_set_header()...\t\t\t");
    smf_message_set_header(msg,TEST_HEADER);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_header()...\t\t\t");
    h = smf_message_get_header(msg,TEST_HEADER_NAME);
    assert(h);
    s = smf_header_to_string(h);
    if (strcmp(s,TEST_HEADER)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);


    g_printf("* testing smf_message_free()...\t\t\t\t");
    smf_message_free(msg);
    g_printf("passed\n");

    return 0;
}
