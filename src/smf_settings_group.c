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
#include <stdlib.h>

#include "smf_settings.h"
#include "smf_trace.h"

#define THIS_MODULE "group_settings"

SMFSettingsGroup_T *smf_settings_group_load(SMFSettings_T *settings, char *group_name) {
	GError *error = NULL;
	SMFSettingsGroup_T *group = g_slice_new(SMFSettingsGroup_T);

	group->data = NULL;
	group->name = g_strdup(group_name);
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}

	/* open config file and start parsing */
	group->data = (void *)g_key_file_new ();
	if (!g_key_file_load_from_file ((GKeyFile *)group->data, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		return NULL;
	}

	if (!g_key_file_has_group((GKeyFile *)group->data,group->name)) {
		TRACE(TRACE_ERR,"config file has no group named %s", group->name);
		return NULL;
	}


	return group;
}

/** Returns the raw value associated with key under the selected group.
 *  Use smf_settings_group_get_string() to retrieve an unescaped UTF-8 string.*/
char *smf_settings_group_get_value(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_value((GKeyFile *)group->data,group->name,key,NULL);
}

/** Returns the string value associated with key under the selected group.
 *  Unlike group_settings_get_value(), this function handles escape
 *  sequences like \s.*/
char *smf_settings_group_get_string(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_string((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the boolean values associated with key under the selected group as integer. */
int smf_settings_group_get_boolean(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_boolean((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the value associated with key under the selected group as an integer. */
int smf_settings_group_get_integer(SMFSettingsGroup_T *group,char *key) {
	return g_key_file_get_integer((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the value associated with key under the selected group as a double. */
double smf_settings_group_get_double(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_double((GKeyFile *)group->data, group->name, key, NULL);
}

/** Returns the values associated with key under the selected group. */
char **smf_settings_group_get_string_list(SMFSettingsGroup_T *group, char *key, int *length) {
	return g_key_file_get_string_list((GKeyFile *)group->data, group->name, key, (gsize*)&length, NULL);
}

/** Free allocated space */
void smf_settings_group_free(SMFSettingsGroup_T *group) {
	g_key_file_free((GKeyFile *)group->data);
	free(group->name);
	g_slice_free(SMFSettingsGroup_T,group);
}
