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

#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_message.h"

#define TEST_SENDER "webmaster@spmfilter.org"
#define TEST_RCPT1 "ast@spmfilter.org"
#define TEST_RCPT2 "postmaster@spmfilter.org"
#define TEST_PATH "/tmp/test.eml"
#define TEST_AUTH_USER "testusername"
#define TEST_AUTH_PASS "testpassword"
#define TEST_NEXTHOP "localhost:2525"

static void print_rcpt_func(SMFEmailAddress_T *ea, void *data) {
	g_printf("\t- [%s]\n",ea->addr);
}

int compare_string(char *s1, char *s2) {
	if (g_strcmp0(s1,s2) != 0) {
		g_printf("\texpecting [%s], but got [%s]",s1,s2);
		return -1;
	}
	
	return 0;
}

int main (int argc, char const *argv[]) {
	SMFMessageEnvelope_T *env = NULL;
	
	g_printf("Start SMFEnvelope_T tests...\n");
	g_printf("* testing smf_message_envelope_new()\n");
	env = smf_message_envelope_new();
	
	g_printf("* testing smf_message_envelope_set_sender() [%s]\n",TEST_SENDER);
	env = smf_message_envelope_set_sender(env,TEST_SENDER);
	if (compare_string(TEST_SENDER,smf_message_envelope_get_sender(env)->addr) != 0)
		return -1;
	
	g_printf("* testing smf_message_envelope_add_rcpt() [%s]\n",TEST_RCPT1);
	env = smf_message_envelope_add_rcpt(env,TEST_RCPT1);
	
	g_printf("* testing smf_message_envelope_add_rcpt() [%s]\n",TEST_RCPT2);
	env = smf_message_envelope_add_rcpt(env,TEST_RCPT2);
	
	g_printf("* testing smf_message_envelope_foreach_rcpt()\n");
	smf_message_envelope_foreach_rcpt(env, print_rcpt_func, NULL);

	
	g_printf("* testing smf_message_envelope_set_message_file() [%s]\n",TEST_PATH);
	env = smf_message_envelope_set_message_file(env,TEST_PATH);
	if (compare_string(TEST_PATH,smf_message_envelope_get_message_file(env)) != 0)
		return -1;

	g_printf("* testing smf_message_envelope_set_auth_user() [%s]\n",TEST_AUTH_USER);
	env = smf_message_envelope_set_auth_user(env,TEST_AUTH_USER);
	if (compare_string(TEST_AUTH_USER,smf_message_envelope_get_auth_user(env)) != 0)
		return -1;
	
	g_printf("* testing smf_message_envelope_set_auth_pass() [%s]\n",TEST_AUTH_PASS);
	env = smf_message_envelope_set_auth_pass(env,TEST_AUTH_PASS);
	if (compare_string(TEST_AUTH_PASS,smf_message_envelope_get_auth_pass(env)) != 0)
		return -1;
	
	g_printf("* testing smf_message_envelope_set_nexthop() [%s]\n",TEST_NEXTHOP);
	env = smf_message_envelope_set_nexthop(env,TEST_NEXTHOP);
	if (compare_string(TEST_NEXTHOP,smf_message_envelope_get_nexthop(env)) != 0)
		return -1;
	
	g_printf("* testing smf_message_envelope_free()\n");
	smf_message_envelope_free(env);
	return 0;
}
