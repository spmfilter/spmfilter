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

#include <glib.h>

#include "spmfilter.h"

#define THIS_MODULE "group_settings"

GKeyFile *keyfile = NULL;
char *group = NULL;

/** Load all settings from specified group to hash table
 *
 * \param group_name wanted group
 *
 * \returns 0 on success or -1 in case of error
 */
int group_settings_load(char *group_name) {
	GError *error = NULL;
	Settings_T *settings = get_settings();

	group = g_strdup(group_name);
	
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}

	/* open config file and start parsing */
	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		return -1;
	}

	if (!g_key_file_has_group(keyfile,group_name)) {
		TRACE(TRACE_ERR,"config file has no group named %s", group_name);
		return -1;
	}

	return 0;
}

char *group_settings_get_string(char *key) {
	return g_key_file_get_string(keyfile, group, key,NULL);
}

int group_settings_get_boolean(char *key) {
	return g_key_file_get_boolean(keyfile, group, key,NULL);
}

int group_settings_get_integer(char *key) {
	return g_key_file_get_integer(keyfile, group, key,NULL);
}

double group_settings_get_double(char *key) {
	return g_key_file_get_double(keyfile, group, key, NULL);
}

char **group_settings_get_string_list(char *key, int *length) {
	return g_key_file_get_string_list(keyfile, group, key, &length, NULL);
}



/** Free allocated space */
void group_settings_free(void) {
	g_key_file_free(keyfile);
}
