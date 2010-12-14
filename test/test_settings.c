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

#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

int main (int argc, char const *argv[]) {
	SMFSettings_T *settings = NULL;
		
	g_thread_init(NULL);
	
	if (!g_thread_supported()) {
		g_printf("glib2 does not support threads!\n");
		return -1;
	} else {
		g_printf("Start SMFSettings_T tests...\n");
		g_printf("* testing smf_settings_new()\n");
		smf_settings_new();		
	}
		
	if (smf_settings_parse_config() != 0)
		return -1;
		
	g_printf("* testing smf_settings_free()\n");
	smf_settings_free();
	
	g_thread_exit(NULL);
	return 0;
}