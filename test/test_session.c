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

#include "../src/smf_session.h"

int main (int argc, char const *argv[]) {
	SMFSession_T *session = NULL;
	
	g_printf("Start SMFSession_T tests...\n");
	g_printf("* testing smf_session_new()\n");
	session = smf_session_new();
	
	g_printf("* testing smf_session_free()\n");
	smf_session_free(session);
	return 0;
}