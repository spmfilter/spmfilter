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

/** Load all settings from specified group to hash table
 *
 * \param group_name wanted group
 *
 * \returns GroupSettings_T
 */
GroupSettings_T *group_settings_load(char *group_name) {
	GroupSettings_T *gs = NULL;
	GError *error = NULL;
	GKeyFile *keyfile;
	char **keys;
	gsize l;
	int i;
	Settings_T *settings = get_settings();

	gs = (GroupSettings_T *)g_hash_table_new((GHashFunc)g_int_hash,(GEqualFunc)g_int_equal);
	
	if (settings->config_file == NULL) {
		settings->config_file = "/etc/spmfilter.conf";
	}

	/* open config file and start parsing */
	keyfile = g_key_file_new ();
	if (!g_key_file_load_from_file (keyfile, settings->config_file, G_KEY_FILE_NONE, &error)) {
		TRACE(TRACE_ERR,"Error loading config: %s",error->message);
		g_error_free(error);
		return NULL;
	}

	if (!g_key_file_has_group(keyfile,group_name)) {
		TRACE(TRACE_ERR,"config file has no group named %s", group_name);
		return NULL;
	}

	keys = g_key_file_get_keys(keyfile,group_name,&l,NULL);
	for (i=0; i < l; i++) {
		char *value;
		value = g_key_file_get_value(keyfile,group_name,keys[i],NULL);
		g_hash_table_insert((GHashTable *)gs,g_strdup(keys[i]), g_strdup(value));
		TRACE(TRACE_DEBUG,"added key [%s] with value [%s]",keys[i],value);
		g_free(value);
	}

	g_strfreev(keys);
	g_key_file_free(keyfile);
	return gs;
}

/** Get value from group
 *
 * \param key key to look for
 *
 * \returns value for key
 */
char *group_settings_get(GroupSettings_T *s, char *key) {
	return (char *)g_hash_table_lookup((GHashTable *)s,key);
}

/** Free allocated space
 *
 * \param s GroupSettings_T
 */
void group_settings_free(GroupSettings_T *s) {
	g_hash_table_destroy((GHashTable *)s);
}
