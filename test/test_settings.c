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
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#define TEST_ENGINE "smtpd"

int main (int argc, char const *argv[]) {
	SMFSettings_T *settings = NULL;
	SMFSettingsGroup_T *test_group = NULL;
	char *s = NULL;
	char **sl = NULL;
	int sl_length = 0;
	
	g_thread_init(NULL);
	
	if (!g_thread_supported()) {
		g_printf("glib2 does not support threads!\n");
		return -1;
	} else {
		g_printf("Start SMFSettings_T tests...\n");
		g_printf("* testing smf_settings_get()...\t\t\t\t");
		settings = smf_settings_get();		
		if (settings != NULL)
			g_printf("passed\n");
		else {
			g_printf("failed\n");
			return -1;
		}
	}
		
	g_printf("* testing smf_settings_parse_config()...\t\t");
	if (smf_settings_parse_config(settings,"../../spmfilter.conf.sample") != 0) {
		g_printf("failed\n");
		return -1;
	} else 
		g_printf("passed\n");
	
	g_printf("* testing smf_settings_group_load()...\t\t\t");
	test_group = smf_settings_group_load(settings, "global");
	if (test_group != NULL) { 
		g_printf("passed\n");
	} else {
		g_printf("failed\n");
		return 0;
	}
		
	
	g_printf("* testing smf_settings_group_get_string()...\t\t");
	s = smf_settings_group_get_string(test_group,"engine");
	if (g_strcmp0(s,TEST_ENGINE) != 0) {
		g_printf("failed\n");
		if (s!=NULL)
			free(s);
		return -1;
	} else {
		g_printf("passed\n");
		free(s);
	}
	
	g_printf("* testing smf_settings_group_get_value()...\t\t");
	s = (char *)smf_settings_group_get_value(test_group,"engine");
	if (g_strcmp0(s,TEST_ENGINE) != 0) {
		g_printf("failed\n");
		if (s!=NULL)
			free(s);
		return -1;
	} else {
		g_printf("passed\n");
		free(s);
	}
	
	g_printf("* testing smf_settings_group_get_boolean()...\t\t");
	if (smf_settings_group_get_boolean(test_group,"add_header")) {
		g_printf("passed\n");
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_group_get_integer()...\t\t");
	if (smf_settings_group_get_integer(test_group,"module_fail") != 3) {
		g_printf("failed\n");
		return -1;
	} else
		g_printf("passed\n");
	
	g_printf("* testing smf_settings_group_get_double()...\t\t");
	if (smf_settings_group_get_double(test_group,"max_size") != 0) {
		g_printf("failed\n");
		return -1;
	} else
		g_printf("passed\n");
	
	g_printf("* testing smf_settings_group_get_string_list()...\t");
	sl = smf_settings_group_get_string_list(test_group,"modules",&sl_length);
	if (sl != NULL) {
		if (g_strcmp0((char *)sl[0],"clamav") != 0) {
			g_printf("failed\n");
			return -1;
		}
		if (g_strcmp0((char *)sl[1],"spamassassin") != 0) {
			g_printf("failed\n");
			return -1;
		}
		g_printf("passed\n");
		g_strfreev(sl);
	} else {
		g_printf("failed\n");
		return -1;
	}
	
	g_printf("* testing smf_settings_group_free()...\t\t\t");
	smf_settings_group_free(test_group);
	g_printf("passed\n");
	
	
	g_printf("* testing smf_settings_free()...\t\t\t");
	smf_settings_free(settings);
	g_printf("passed\n");
	
	g_thread_exit(NULL);
	return 0;
}