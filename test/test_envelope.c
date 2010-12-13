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
#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_message.h"

#define TEST_SENDER "webmaster@spmfilter.org"
#define TEST_RCPT "ast@spmfilter.org"

int main (int argc, char const *argv[]) {
	SMFMessageEnvelope_T * env = NULL;
	char *sender = NULL;
	char *rcpt = NULL;
	
	sender = g_strdup(TEST_SENDER);
	rcpt = g_strdup(TEST_RCPT);
	
	env = smf_message_envelope_new();
	
	env = smf_message_envelope_set_sender(env,sender);
	g_printf("Setting envelope sender to [%s]\n",smf_message_envelope_get_sender(env)->addr);
	
	g_printf("Adding rcpt [%s] to envelope\n",rcpt);
	env = smf_message_envelope_add_rcpt(env,rcpt);
	
	g_free(sender);
	g_free(rcpt);
	smf_message_envelope_free(env);
	return 0;
}
