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

#define TEST_ADDR "foo@bar.com"

int main (int argc, char const *argv[]) {
    SMFMessage_T *msg = NULL;
    char *s = NULL;

    g_printf("Start SMFMessage_T tests...\n");

    g_printf("* testing smf_message_new()...\t\t\t\t");
    msg = smf_message_new();
    assert(msg);
    g_printf("passed\n");
  
    g_printf("* testing smf_message_set_sender()...\t\t\t");
    smf_message_set_sender(msg,TEST_ADDR);
    g_printf("passed\n");
    g_printf("* testing smf_message_get_sender()...\t\t\t");
    s = cmime_message_get_sender_string(msg);
    assert(strcmp(s,TEST_ADDR)==0);
    free(s);  

    g_printf("* testing smf_message_free()...\t\t\t\t");
    smf_message_free(msg);
    g_printf("passed\n");

    return 0;
}
