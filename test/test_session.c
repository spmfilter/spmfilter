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

#include "../src/smf_session.h"
#include "../src/smf_session_private.h"

#define TEST_HELO "foo.bar"
#define TEST_XFWD "127.0.0.1"
#define TEST_RESPONSE "250 OK message accepted"

int main (int argc, char const *argv[]) {
    SMFSession_T *session = NULL;
    
    g_printf("Start SMFSession_T tests...\n");

    g_printf("* testing smf_session_new()...\t\t\t\t");
    session = smf_session_new();
    assert(session);
    g_printf("passed\n");
    
    g_printf("* testing smf_session_set_helo()...\t\t\t");
    smf_session_set_helo(session,TEST_HELO);

    if (strcmp(TEST_HELO,smf_session_get_helo(session)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
      
    g_printf("* testing smf_session_set_xfoward_addr()...\t\t");
    smf_session_set_xforward_addr(session,TEST_XFWD);
    if (strcmp(TEST_XFWD,smf_session_get_xforward_addr(session)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");

    g_printf("* testing smf_session_set_response_msg()...\t\t");
    smf_session_set_response_msg(session,TEST_RESPONSE);

    if (strcmp(TEST_RESPONSE,smf_session_get_response_msg(session)) != 0) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
  
    g_printf("* testing smf_session_free()...\t\t\t\t");
    smf_session_free(session);
    g_printf("passed\n");

    return 0;
}
