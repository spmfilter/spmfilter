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

#ifndef _SMF_SETTINGS_H
#define	_SMF_SETTINGS_H

typedef enum {
	SMF_TLS_DISABLED,
	SMF_TLS_ENABLED,
	SMF_TLS_REQUIRED
} SMFTlsOption_T;

typedef struct {
	/* debug flag */
	int debug;

	/* path to config file */
	char *config_file;

	/* path to spool directory */
	char *queue_dir;

	/* configured engine */
	char *engine;

	/* all configured modules */
	char **modules;

	/* behavior when module fails */
	int module_fail;

	/* next smtp hop */
	char *nexthop;

	/* smtp code, when delivery to
	 * nexthop fails */
	int nexthop_fail_code;

	/* smtp return message, when delivery
	 * to nexthop fails */
	char *nexthop_fail_msg;

	/* configured lookup backend */
	char **backend;

	/* if multiple backend hosts are defined,
	 * it's possible to balance connections
	 * between all, or do failover connections.
	 * possible keys are:
	 * - balance
	 *   load-balance connections among all hosts listed
	 * - failover
	 *   failover connections in the order listed
	 */
	char *backend_connection;

	/* add spmfilter processing header */
	int add_header;

	/* maximal message size in bytes */
	unsigned long max_size;

	/* enable/disable TLS */
	SMFTlsOption_T tls;

	/* password for ssl cert */
	char *tls_pass;

        /* daemonize? */
        int daemon;

        /* daemon bind ip */
        char *bind_ip;

        /* daemon port */
        int bind_port;

	/* zdb settings */
	char *sql_driver;
	char *sql_name;
	char **sql_host;
	int sql_num_hosts;
	int sql_port;
	char *sql_user;
	char *sql_pass;
	char *sql_user_query;
	char *sql_encoding;
	int sql_max_connections;

	/* ldap settings */
	char *ldap_uri;
	char **ldap_host;
	int ldap_num_hosts;
	int ldap_port;
	char *ldap_binddn;
	char *ldap_bindpw;
	char *ldap_base;
	int ldap_referrals;
	char *ldap_scope;
	char *ldap_user_query;
} SMFSettings_T;

/** Get all core settings
 *
 * \return settings struct
 */
SMFSettings_T *smf_settings_get(void);

/** Load all settings from specified group
 *
 * \param group_name wanted group
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_group_load(char *group_name);

/** Returns the raw value associated with key under the selected group.
 *  Use smf_settings_group_get_string() to retrieve an unescaped UTF-8 string.
 *
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_value(char *key);

/** Returns the string value associated with key under the selected group.
 *  Unlike group_settings_get_value(), this function handles escape
 *  sequences like \s.
 *
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_string(char *key);

/** Returns the boolean values associated with key under the selected group as integer.
 *
 * \param key a key
 *
 * \returns the value associated with the key as a integer, or 0 if the key was
 * not found or could not be parsed.
 */
int smf_settings_group_get_boolean(char *key);

/** Returns the value associated with key under the selected group as an integer.
 *
 * \param key a key
 *
 * \returns he value associated with the key as an integer, or 0 if the key was not found or could not be parsed.
 */
int smf_settings_group_get_integer(char *key);

/** Returns the value associated with key under the selected group as a double.
 *
 * \param key a key
 *
 * \returns the value associated with the key as a double, or 0.0 if the key was not found or could not be parsed.
 */
double smf_settings_group_get_double(char *key);

/** Returns the values associated with key under the selected group.
 *
 * \param key a key
 *
 * \returns a NULL-terminated string array or NULL if the specified key cannot be found.
 */
char **smf_settings_group_get_string_list(char *key, int length);

/** Free allocated space
 *
 * \param s GroupSettings_T
 */
void smf_settings_group_free(void);

#endif	/* _SMF_SETTINGS_H */
