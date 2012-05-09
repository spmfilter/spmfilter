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

#define TEST_ADDR "webmaster@spmfilter.org"

int main (int argc, char const *argv[]) {
	SMFEmailAddress_T *ea = NULL;
	
	g_printf("Start SMFEmailAddress_T tests...\n");

	g_printf("* testing smf_email_address_new()...\t\t\t\t");
	ea = smf_email_address_new();
	g_printf("passed\n");
	
	g_printf("* testing smf_email_address_set_addr()...\t\t\t");
	ea = smf_email_address_set_addr(ea,TEST_ADDR);
	
	if (strcmp(TEST_ADDR,smf_email_address_get_addr(ea)) != 0) {
		g_printf("failed\n");
		return -1;
	} else
		g_printf("passed\n");
		
	g_printf("* testing smf_email_address_free()...\t\t\t\t");
	smf_email_address_free(ea);
	g_printf("passed\n");
	
	return 0;
}
