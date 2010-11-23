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

/** Load all settings from specified group
 *
 * \param group_name wanted group
 *
 * \returns SMFSettingsGroup_T object
 */
SMFSettingsGroup_T *smf_settings_group_load(char *group_name) {
	GError *error = NULL;
	SMFSettingsGroup_T *group = g_slice_new(SMFSettingsGroup_T);
	SMFSettings_T *settings = smf_settings_get();

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
 *  Use smf_settings_group_get_string() to retrieve an unescaped UTF-8 string.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_value(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_value((GKeyFile *)group->data,group->name,key,NULL);
}

/** Returns the string value associated with key under the selected group.
 *  Unlike group_settings_get_value(), this function handles escape
 *  sequences like \s.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_string(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_string((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the boolean values associated with key under the selected group as integer.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns the value associated with the key as a integer, or 0 if the key was
 * not found or could not be parsed.
 */
int smf_settings_group_get_boolean(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_boolean((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the value associated with key under the selected group as an integer.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns he value associated with the key as an integer, or 0 if the key was not found or could not be parsed.
 */
int smf_settings_group_get_integer(SMFSettingsGroup_T *group,char *key) {
	return g_key_file_get_integer((GKeyFile *)group->data, group->name, key,NULL);
}

/** Returns the value associated with key under the selected group as a double.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns the value associated with the key as a double, or 0.0 if the key was not found or could not be parsed.
 */
double smf_settings_group_get_double(SMFSettingsGroup_T *group, char *key) {
	return g_key_file_get_double((GKeyFile *)group->data, group->name, key, NULL);
}

/** Returns the values associated with key under the selected group.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 * \param length return location for the number of returned strings, or NULL
 *
 * \returns a NULL-terminated string array or NULL if the specified key cannot be found.
 */
char **smf_settings_group_get_string_list(SMFSettingsGroup_T *group, char *key, int length) {
	return g_key_file_get_string_list((GKeyFile *)group->data, group->name, key, (gsize*)&length, NULL);
}

/** Free allocated space
 *
 * \param s GroupSettings_T
 */
void smf_settings_group_free(SMFSettingsGroup_T *group) {
	g_key_file_free((GKeyFile *)group->data);
	free(group->name);
	g_slice_free(SMFSettingsGroup_T,group);
}
