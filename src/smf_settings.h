/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner and SpaceNet AG
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

/*!
 * @file smf_settings.h
 * @brief Defines spmfilter configuration and config file parsing functions
 */

#ifndef _SMF_SETTINGS_H
#define _SMF_SETTINGS_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "spmfilter_config.h"
#include "smf_smtp_codes.h"
#include "smf_trace.h"

/*!
 * @enum SMFTlsOption_T
 * @brief Possible types of TLS configuration 
 */
typedef enum {
    SMF_TLS_DISABLED, /**< TLS is disabled */
    SMF_TLS_ENABLED, /**< TLS is enabled */
    SMF_TLS_REQUIRED /**< TLS is enabled and required */
} SMFTlsOption_T;

/*!
 * @struct SMFSettings_T smf_settings.h
 * @brief Holds spmfilter runtime configuration 
 */
typedef struct {
    int debug; /**< debug flag */
    char *config_file; /**< path to config file */
    char *queue_dir; /**< path to spool directory */
    char *engine; /**< configured engine */
    char **modules; /**< all configured modules */
    int module_fail; /**< module fail behavior */
    char *nexthop; /**< next smtp hop */
    int nexthop_fail_code; /**< smtp code, when delivery to nexthop fails */
    char *nexthop_fail_msg; /**< smtp return message, when delivery to nexthop fails */
    char **backend; /**< configured lookup backend */
    char *backend_connection; /**< if multiple backend hosts are defined,
                               * it's possible to balance connections
                               * between all, or do failover connections.
                               * possible keys are:
                               * - balance
                               *   load-balance connections among all hosts listed
                               * - failover
                               *   failover connections in the order listed
                               */
    int add_header; /**< add spmfilter processing header */
    unsigned long max_size; /**< maximal message size in bytes */
    SMFTlsOption_T tls; /**< enable/disable TLS */
    char *tls_pass; /**< password for ssl cert */
    int daemon; /**< daemonize flag */

    char *sql_driver; /**< sql driver name */
    char *sql_name; /**< sql database name */
    char **sql_host; /**< list with sql database hosts */
    int sql_num_hosts; /**< number of sql database hosts */
    int sql_port; /**< sql database port */
    char *sql_user; /**< sql database username */
    char *sql_pass; /**< sql database password */
    char *sql_user_query; /**< sql user query */
    char *sql_encoding; /**< sql encoding */
    int sql_max_connections; /**< max. number of sql conncetions */

    char *ldap_uri; /**< ldap uri */
    char **ldap_host; /**< list with ldap hosts */
    int ldap_num_hosts; /**< number of ldap hosts */
    int ldap_port; /**< ldap port */
    char *ldap_binddn; /**< ldap bind dn */
    char *ldap_bindpw; /**< ldap bind password */
    char *ldap_base; /**< ldap search base */
    int ldap_referrals; /**< ldap referrals flag */
    char *ldap_scope; /**< ldap search scope */
    char *ldap_user_query; /**< ldap user query */
} SMFSettings_T;

/*!
 * @struct SMFSettingsGroup_T smf_settings.h
 * @brief Represents a config file settings group
 */
typedef struct {
    char *name; /**< name of the setting group */
    void *data; /**< setting group values */
} SMFSettingsGroup_T;

/* has to be removed */
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

/** Set path to config file
 *
 * \param cf config file
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_set_config_file(char *cf);

/** Get config file path
 *
 * \returns config file path
 */
char *smf_settings_get_config_file(void);

/** Set path to queue directory
 *
 * \param qd queue directory path
 *
 * \returns 0 on success or -1 in case of error
 */
int smf_settings_set_queue_dir(char *qd);

/** Get queue directory path
 *
 * \returns queue directory
 */
char *smf_settings_get_queue_dir(void);

/** Set engine which should be used
 *
 * \param engine engine
 */
void smf_settings_set_engine(char *engine);

/** Get configured engine
 *
 * \returns engine
 */
char *smf_settings_get_engine(void);

/** Set available modules, which will be loaded at runtime.
 * 
 * \param modules list with available modules
 */
void smf_settings_set_modules(char **modules);

/** Get available modules
 * 
 * \returns modules list
 */
char **smf_settings_get_modules(void);

/** Set module_fail setting
 *  1 = proceed and ignore
 *  2 = cancel further processing and return permanet error
 *  3 = cancel further processing and return temporary error (default) 
 *
 * \param i module_fail value
 */
void smf_settings_set_module_fail(int i);

/** Get module_fail setting
 *
 * \returns module_fail setting
 */
int smf_settings_get_module_fail(void);

/** Set nexthop setting.
 *  This parameter specifies the final destination, 
 *  after a mail is processed by spmfilter.
 *
 * \params nexthop nexthtop string
 */
void smf_settings_set_nexthop(char *nexthop);

/** Get configured nexthop setting
 *
 * \returns nexthop
 */
char *smf_settings_get_nexthop(void);

/** Set nexthop_fail_code setting
 *  If the delivery to the final destination fails 
 *  for any reason, this code is used as response to 
 *  the sending  MTA (default 451).  
 *
 * \param i nexthop_fail_code value
 */
void smf_settings_set_nexthop_fail_code(int i);

/** Get nexthop_fail_code setting
 *
 * \returns nexthop_fail_code
 */
int smf_settings_get_nexthop_fail_code(void);

/** Set nexthop_fail_msg setting.
 *  If the delivery to the final destination fails for any 
 *  reason, this message is used as reponse for the sending
 *  MTA. (default "Requested action aborted: local error in processing").
 *
 * \param msg nexthop fail message
 */
void smf_settings_set_nexthop_fail_msg(char *msg);

/** Get nexthop_fail_msg setting
 *
 * \returns nexthop fail message
 */
char *smf_settings_get_nexthop_fail_msg(void);

/** Set lookup backend. 
 *
 * \param backend backend setting list
 */
void smf_settings_set_backend(char **backend);

/** Get backend setting
 *
 * \returns backend setting list 
 */
char **smf_settings_get_backend(void);

/** Set backend_connection setting.
 *  If there are multiple server configured in the specified backend, 
 *  it's possible to define a failover or load-balancing behaviour. 
 *  Possible values are:
 *    balance  = when you configure the backend profile for load
 *               balancing, spmfilter distributes connections across
 *               the list of hosts. If the actual host is not reachable,
 *               spmfilter switches back to failover configuration.
 *    failover = when you configure the backend profile for
 *               failover, spmfilter fails over to the next host in
 *               the list if it cannot connect to the first host.
 *
 * \param conn backend connection
 */
void smf_settings_set_backend_connection(char *conn);

/** Get backend connection setting
 *
 * \returns backend connection 
 */
char *smf_settings_get_backend_connection(void);

/** Define if spmfilter should add it's own header
 *
 * \param i add_header value, either 1 (true) or 0 (false)
 */
void smf_settings_set_add_header(int i);

/** Get add_header setting
 *
 * \returns add_header value
 */
int smf_settings_get_add_header(void);

/** Set max. allowed message size in byte
 *
 * \param size max_size setting
 */
void smf_settings_set_max_size(unsigned long size);

/** Get max_size setting in bytes
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

/** Set passphrase for SSL certificate, if needed.
 *
 * \param pass tls password
 */
void smf_settings_set_tls_pass(char *pass);

/** Get tls_pass setting
 *
 * \returns tls password
 */
char *smf_settings_get_tls_pass(void);

/** Set daemon setting.
 *  Define whether spmfilter should run in 
 *  threaded daemon mode, or not.
 *
 * \param i daemon value
 */
void smf_settings_set_daemon(int i);

/** Get daemon setting
 *
 * \returns daemon value
 */
int smf_settings_get_daemon(void);

/** Set SQL driver, which should be used.
 *  Possible values are:
 *  - mysql
 *  - postgresql
 *  - sqlite
 *
 * \param driver sql_driver value
 */
void smf_settings_set_sql_driver(char *driver);

/** Get sql_driver setting
 *
 * \returns sql_driver value
 */
char *smf_settings_get_sql_driver(void);

/** Set SQL database name
 *
 * \param name database name
 */
void smf_settings_set_sql_name(char *name);

/** Get SQL database name 
 *
 * \returns sql_name value
 */
char *smf_settings_get_sql_name(void);

/** Set SQL host(s)
 *
 * \param host sql_host list
 */
void smf_settings_set_sql_host(char **host);

/** Get SQL host(s)
 *
 * \returns sql_host list
 */
char **smf_settings_get_sql_host(void);

/** Get number of configured SQL hosts
 *
 * \returns sql_num_hosts value
 */
int smf_settings_get_sql_num_hosts(void);

/** Set SQL port
 *
 * \param port sql port
 */
void smf_settings_set_sql_port(int port);

/** Get SQL port
 *
 * \returns sql_port value
 */
int smf_settings_get_sql_port(void);

/** Set SQL username
 *
 * \param user sql user value
 */
void smf_settings_set_sql_user(char *user);

/** Get SQL username
 *
 * \returns sql_user value
 */
char *smf_settings_get_sql_user(void);

/** Set SQL password
 *
 * \param pass sql_pass value
 */
void smf_settings_set_sql_pass(char *pass);

/** Get SQL password
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

/** Set SQL encoding
 *
 * \param encoding sql encoding
 */
void smf_settings_set_sql_encoding(char *encoding);

/** Get SQL encoding
 *
 * \returns sql encoding
 */
char *smf_settings_get_sql_encoding(void);

/** Set max. number of SQL connections
 *
 * \param i number of max allowed connections
 */
void smf_settings_set_sql_max_connections(int i);

/** Get max. number of SQL connections
 *
 * \returns number of max. connections
 */
int smf_settings_get_sql_max_connections(void);

/** Set LDAP uri
 *
 * \param uri ldap uri value
 */
void smf_settings_set_ldap_uri(char *uri);

/** Get LDAP uri
 *
 * \returns ldap uri value
 */
char *smf_settings_get_ldap_uri(void);

/** Set LDAP host(s)
 *
 * \param host ldap host list
 */
void smf_settings_set_ldap_host(char **host);

/** Get LDAP host(s)
 *
 * \returns ldap host list
 */
char **smf_settings_get_ldap_host(void);

/** Get number of configured LDAP hosts
 *
 * \returns ldap_num_hosts value
 */
int smf_settings_get_ldap_num_hosts(void);

/** Set LDAP port
 *
 * \param port ldap port value 
 */
void smf_settings_set_ldap_port(int port);

/** Get LDAP port
 *
 * \returns ldap_port value
 */
int smf_settings_get_ldap_port(void);

/** Set LDAP binddn
 * 
 * \param binddn ldap binddn value
 */
void smf_settings_set_ldap_binddn(char *binddn);

/** Get LDAP binddn
 *
 * \returns ldap binddn value
 */
char *smf_settings_get_ldap_binddn(void);

/** Set LDAP bind password
 *
 * \param bindpw ldap bindpw value
 */
void smf_settings_set_ldap_bindpw(char *bindpw);

/** Get LDAP bind password
 * 
 * \returns ldap_bindpw value
 */
char *smf_settings_get_ldap_bindpw(void);

/** Set LDAP search base
 *
 * \param base ldap base value
 */
void smf_settings_set_ldap_base(char *base);

/** Get LDAP search base
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

/** Set LDAP scope 
 *  Possible values are:
 *  - subtree
 *  - onelevel
 *  - base
 *
 * \param scope ldap scope value
 */
void smf_settings_set_ldap_scope(char *scope);

/** Get LDAP scope 
 *
 * \returns ldap scope
 */
char *smf_settings_get_ldap_scope(void);

/** Set LDAP user query
 * 
 * \param query ldap user query
 */
void smf_settings_set_ldap_user_query(char *query);

/** Get LDAP user query
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

#endif  /* _SMF_SETTINGS_H */
