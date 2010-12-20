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

typedef struct {
	char *name;
	void *data;
} SMFSettingsGroup_T;

/** Get all core settings
 *
 * \return settings struct
*/
SMFSettings_T *smf_settings_get(void);

void smf_settings_set_debug(int debug);
int smf_settings_get_debug(void);

void smf_settings_set_config_file(char *cf);
char *smf_settings_get_config_file(void);

void smf_settings_set_queue_dir(char *qd);
char *smf_settings_get_queue_dir(void);

void smf_settings_set_engine(char *engine);
char *smf_settings_get_engine(void);

void smf_settings_set_modules(char **modules);
char **smf_settings_get_modules(void);

void smf_settings_set_module_fail(int i);
int smf_settings_get_module_fail(void);

void smf_settings_set_nexthop(char *nexthop);
char *smf_settings_get_nexthop(void);

void smf_settings_set_nexthop_fail_code(int i);
int smf_settings_get_nexthop_fail_code(void);

void smf_settings_set_nexthop_fail_msg(char *msg);
char *smf_settings_get_nexthop_fail_msg(void);

void smf_settings_set_backend(char **backend);
char **smf_settings_get_backend(void);

void smf_settings_set_backend_connectio(char *conn);
char *smf_settings_get_backend_connection(void);

void smf_settings_set_add_header(int i);
int smf_settings_get_add_header(void);

void smf_settings_set_max_size(unsigned long size);
unsigned long smf_settings_get_max_size(void);

void smf_settings_set_tls(SMFTlsOption_T t);
SMFTlsOption_T smf_settings_get_tls(void);

void smf_settings_set_tls_pass(char *pass);
char *smf_settings_get_tls_pass(void);

void smf_settings_set_daemon(int i);
int smf_settings_get_daemon(void);

void smf_settings_set_sql_driver(char *driver);
char *smf_settings_get_sql_driver(void);

void smf_settings_set_sql_name(char *name);
char *smf_settings_get_sql_name(void);

void smf_settings_set_sql_host(char **host);
char **smf_settings_get_sql_host(void);

int smf_settings_get_sql_num_hosts(void);

void smf_settings_set_sql_port(int port);
int smf_settings_get_sql_port(void);

void smf_settings_set_sql_user(char *user);
char *smf_settings_get_sql_user(void);

void smf_settings_set_sql_pass(char *pass);
char *smf_settings_get_sql_pass(void);

void smf_settings_set_sql_user_query(char *query);
char *smf_settings_get_sql_user_query(void);

void smf_settings_set_sql_encoding(char *encoding);
char *smf_settings_get_sql_encoding(void);

void smf_settings_set_sql_max_connections(int i);
int smf_settings_get_sql_max_connections(void);

void smf_settings_set_ldap_uri(char *uri);
char *smf_settings_get_ldap_uri(void);

void smf_settings_set_ldap_host(char **host);
char **smf_settings_get_ldap_host(void);

int smf_settings_get_ldap_num_hosts(void);

void smf_settings_set_ldap_port(int port);
int smf_settings_get_ldap_port(void);

void smf_settings_set_ldap_binddn(char *binddn);
char *smf_settings_get_ldap_binddn(void);

void smf_settings_set_ldap_bindpw(char *bindpw);
char *smf_settings_get_ldap_bindpw(void);

void smf_settings_set_ldap_base(char *base);
char *smf_settings_get_ldap_base(void);

void smf_settings_set_ldap_referrals(int i);
int smf_settings_get_ldap_referrals(void);

void smf_settings_set_ldap_scope(char *scope);
char *smf_settings_get_ldap_scope(void);

void smf_settings_set_ldap_user_query(char *query);
char *smf_settings_get_ldap_user_query(void);

/** Load all settings from specified group
 *
 * \param SMFSettings_T object
 * \param group_name wanted group
 *
 * \returns SMFSettingsGroup_T object
 */
SMFSettingsGroup_T *smf_settings_group_load(SMFSettings_T* settings, char *group_name);

/** Returns the raw value associated with key under the selected group.
 *  Use smf_settings_group_get_string() to retrieve an unescaped UTF-8 string.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_value(SMFSettingsGroup_T *group, char *key);

/** Returns the string value associated with key under the selected group.
 *  Unlike group_settings_get_value(), this function handles escape
 *  sequences like \s.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_string(SMFSettingsGroup_T *group, char *key);

/** Returns the boolean values associated with key under the selected group as integer.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns the value associated with the key as a integer, or 0 if the key was
 * not found or could not be parsed.
 */
int smf_settings_group_get_boolean(SMFSettingsGroup_T *group, char *key);

/** Returns the value associated with key under the selected group as an integer.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns he value associated with the key as an integer, or 0 if the key was not found or could not be parsed.
 */
int smf_settings_group_get_integer(SMFSettingsGroup_T *group,char *key);

/** Returns the value associated with key under the selected group as a double.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 *
 * \returns the value associated with the key as a double, or 0.0 if the key was not found or could not be parsed.
 */
double smf_settings_group_get_double(SMFSettingsGroup_T *group, char *key);

/** Returns the values associated with key under the selected group.
 *
 * \param group a SMFSettingsGroup_T object
 * \param key a key
 * \param length return location for the number of returned strings, or NULL
 *
 * \returns a NULL-terminated string array or NULL if the specified key cannot be found.
 */
char **smf_settings_group_get_string_list(SMFSettingsGroup_T *group, char *key, int *length);

/** Free allocated space
 *
 * \param s GroupSettings_T
 */
void smf_settings_group_free(SMFSettingsGroup_T *group);

#endif	/* _SMF_SETTINGS_H */
