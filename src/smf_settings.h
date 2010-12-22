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
 * \returns settings struct
*/
SMFSettings_T *smf_settings_get(void);

/** Set debug setting
 *
 * \param debug debug setting  either 0 (false) or 1 (true)
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_set_debug(int debug);

/** Get debug setting
 * 
 * \returns debug setting
 */
int smf_settings_get_debug(void);

/** Set config file setting
 *
 * \param cf config file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_set_config_file(char *cf);

/** Get config file settings
 *
 * \returns config file path
 */
char *smf_settings_get_config_file(void);

/** Set queue dir setting
 *
 * \param qd queue directory path
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_set_queue_dir(char *qd);

/** Get queue directory settings
 *
 * \returns queue directory
 */
char *smf_settings_get_queue_dir(void);

/** Set engine setting
 *
 * \param engine engine
 */
void smf_settings_set_engine(char *engine);

/** Get engine setting
 *
 * \returns engine
 */
char *smf_settings_get_engine(void);

/** Set modules setting 
 * 
 * \param modules modules list
 */
void smf_settings_set_modules(char **modules);

/** Get modules setting
 * 
 * \returns modules list
 */
char **smf_settings_get_modules(void);

/** Set module_fail setting
 *
 * \param i module_fail value
 */
void smf_settings_set_module_fail(int i);

/** Get module_fail setting
 *
 * \returns module_fail setting
 */
int smf_settings_get_module_fail(void);

/** Set nexthop setting
 *
 * \params nexthop nexthtop string
 */
void smf_settings_set_nexthop(char *nexthop);

/** Get nexthop setting
 *
 * \returns nexthop
 */
char *smf_settings_get_nexthop(void);

/** Set nexthop_fail_code setting
 *
 * \param i nexthop_fail_code value
 */
void smf_settings_set_nexthop_fail_code(int i);

/** Get nexthop_fail_code setting
 *
 * \returns nexthop_fail_code
 */
int smf_settings_get_nexthop_fail_code(void);

/** Set nexthop_fail_msg setting
 *
 * \param msg nexthop fail message
 */
void smf_settings_set_nexthop_fail_msg(char *msg);

/** Get nexthop_fail_msg setting
 *
 * \returns nexthop fail message
 */
char *smf_settings_get_nexthop_fail_msg(void);

/** Set backend setting
 *
 * \param backend backend setting list
 */
void smf_settings_set_backend(char **backend);

/** Get backend setting
 *
 * \returns backend setting list 
 */
char **smf_settings_get_backend(void);

/** Set backend_connection setting
 *
 * \param conn backend connection
 */
void smf_settings_set_backend_connection(char *conn);

/** Get backend connection
 *
 * \returns backend connection */
char *smf_settings_get_backend_connection(void);

/** Set add_header setting
 *
 * \param i add_header value
 */
void smf_settings_set_add_header(int i);

/** Get add_header setting
 *
 * \returns add_header value
 */
int smf_settings_get_add_header(void);

/** Set max_size setting
 *
 * \param size max_size setting
 */
void smf_settings_set_max_size(unsigned long size);

/** Get max_size setting
 *
 * \returns max_size value
 */
unsigned long smf_settings_get_max_size(void);

/** Set tls setting
 *
 * \param t a SMFTlsOption_T value
 */
void smf_settings_set_tls(SMFTlsOption_T t);

/** Get tls setting
 *
 * \returns a SMFTlsOption_T value
 */
SMFTlsOption_T smf_settings_get_tls(void);

/** Set tls_pass setting
 *
 * \param pass tls password
 */
void smf_settings_set_tls_pass(char *pass);

/** Get tls_pass setting
 *
 * \returns tls password
 */
char *smf_settings_get_tls_pass(void);

/** Set daemon setting
 *
 * \param i daemon value
 */
void smf_settings_set_daemon(int i);

/** Get daemon setting
 *
 * \returns daemon value
 */
int smf_settings_get_daemon(void);

/** Set sql_driver setting
 *
 * \param driver sql_driver value
 */
void smf_settings_set_sql_driver(char *driver);

/** Get sql_driver setting
 *
 * \returns sql_driver value
 */
char *smf_settings_get_sql_driver(void);

/** Set sql_name setting
 *
 * \param name sql_name
 */
void smf_settings_set_sql_name(char *name);

/** Get sql_name setting
 *
 * \returns sql_name value
 */
char *smf_settings_get_sql_name(void);

/** Set sql_host setting
 *
 * \param host sql_host list
 */
void smf_settings_set_sql_host(char **host);

/** Get sql_host setting
 *
 * \returns sql_host list
 */
char **smf_settings_get_sql_host(void);

/** Get sql_num_hosts value
 *
 * \returns sql_num_hosts value
 */
int smf_settings_get_sql_num_hosts(void);

/** Set sql_port setting
 *
 * \param port sql port
 */
void smf_settings_set_sql_port(int port);

/** Get sql_port setting
 *
 * \returns sql_port value
 */
int smf_settings_get_sql_port(void);

/** Set sql_user setting
 *
 * \param user sql user value
 */
void smf_settings_set_sql_user(char *user);

/** Get sql_user setting
 *
 * \returns sql_user value
 */
char *smf_settings_get_sql_user(void);

/** Set sql_pass setting
 *
 * \param pass sql_pass value
 */
void smf_settings_set_sql_pass(char *pass);

/** Get sql_pass setting
 *
 * \returns sql_pass value
 */
char *smf_settings_get_sql_pass(void);

/** Set sql_user_query setting
 *
 * \param query sql user query value
 */
void smf_settings_set_sql_user_query(char *query);

/** Get sql_user_query setting
 *
 * \returns sql user query value
 */
char *smf_settings_get_sql_user_query(void);

/** Set sql_encoding setting
 *
 * \param encoding sql encoding
 */
void smf_settings_set_sql_encoding(char *encoding);

/** Get sql_encoding setting
 *
 * \returns sql encoding
 */
char *smf_settings_get_sql_encoding(void);

/** Set sql_max_connections setting
 *
 * \param i number of max allowed connections
 */
void smf_settings_set_sql_max_connections(int i);

/** Get sql_max_connection setting
 *
 * \returns number of max. connections
 */
int smf_settings_get_sql_max_connections(void);

/** Set ldap_uri setting
 *
 * \param uri ldap uri value
 */
void smf_settings_set_ldap_uri(char *uri);

/** Get ldap_uri setting
 *
 * \returns ldap uri value
 */
char *smf_settings_get_ldap_uri(void);

/** Set ldap_host setting
 *
 * \param host ldap host list
 */
void smf_settings_set_ldap_host(char **host);

/** Get ldap_host setting
 *
 * \returns ldap host list
 */
char **smf_settings_get_ldap_host(void);

/** Get ldap_num_hosts value
 *
 * \returns ldap_num_hosts value
 */
int smf_settings_get_ldap_num_hosts(void);

/** Set ldap_port setting
 *
 * \param port ldap port value 
 */
void smf_settings_set_ldap_port(int port);

/** Get ldap_port setting
 *
 * \returns ldap_port value
 */
int smf_settings_get_ldap_port(void);

/** Set ldap_binddn setting
 * 
 * \param binddn ldap binddn value
 */
void smf_settings_set_ldap_binddn(char *binddn);

/** Get ldap_binddn setting
 *
 * \returns ldap binddn value
 */
char *smf_settings_get_ldap_binddn(void);

/** Set ldap_bindpw setting
 *
 * \param bindpw ldap bindpw value
 */
void smf_settings_set_ldap_bindpw(char *bindpw);

/** Get ldap_bindpw setting
 * 
 * \returns ldap_bindpw value
 */
char *smf_settings_get_ldap_bindpw(void);

/** Set ldap_base setting
 *
 * \param base ldap base value
 */
void smf_settings_set_ldap_base(char *base);

/** Get ldap_base setting
 *
 * \returns ldap base value
 */
char *smf_settings_get_ldap_base(void);

/** Set ldap_referrals setting
 *
 * \param i ldap referrals value
 */
void smf_settings_set_ldap_referrals(int i);

/** Get ldap_referrals setting
 * 
 * \returns ldap referrals value
 */
int smf_settings_get_ldap_referrals(void);

/** Set ldap_scope setting
 *
 * \param scope ldap scope value
 */
void smf_settings_set_ldap_scope(char *scope);

/** Get ldap_scope setting
 *
 * \returns ldap scope
 */
char *smf_settings_get_ldap_scope(void);

/** Set ldap_user_query setting
 * 
 * \param query ldap user query
 */
void smf_settings_set_ldap_user_query(char *query);

/** Get ldap_user_query setting
 * 
 * \returns ldap user_query
 */
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
