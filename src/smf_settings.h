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
#include <assert.h>
#include <glib.h>
#include <glib/gstdio.h>

#include "spmfilter_config.h"
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
    char *lib_dir; /**< user defined directory path for shared libraries */

    GHashTable *smtp_codes; /**< user defined smtp return codes */

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
//SMFSettings_T *smf_settings_get(void); 

/*!
 * @fn int smf_settings_set_debug(SMFSettings_T *settings, int debug)
 * @brief Set debug setting
 * @param settings a SMFSettings_T object
 * @param debug debug setting  either 0 (false) or 1 (true)
 * @returns 0 on success or -1 in case of error
 */
int smf_settings_set_debug(SMFSettings_T *settings, int debug);

/*!
 * @fn int smf_settings_get_debug(SMFSettings_T *settings)
 * @brief Get debug setting
 * @param settings a SMFSettings_T object
 * @returns debug setting
 */
int smf_settings_get_debug(SMFSettings_T *settings);

/*!
 * @fn int smf_settings_set_config_file(SMFSettings_T *settings, char *cf)
 * @brief Set path to config file
 * @param settings a SMFSettings_T object
 * @param cf config file
 * @returns 0 on success or -1 in case of error
 */
int smf_settings_set_config_file(SMFSettings_T *settings, char *cf);

/*!
 * @fn char *smf_settings_get_config_file(SMFSettings_T *settings)
 * @brief Get config file path
 * @param settings a SMFSettings_T object
 * @returns config file path
 */
char *smf_settings_get_config_file(SMFSettings_T *settings);

/*!
 * @fn int smf_settings_set_queue_dir(SMFSettings_T *settings, char *qd)
 * @brief Set path to queue directory
 * @param settings a SMFSettings_T object
 * @param qd queue directory path
 * @returns 0 on success or -1 in case of error
 */
int smf_settings_set_queue_dir(SMFSettings_T *settings, char *qd);

/*!
 * @fn char *smf_settings_get_queue_dir(SMFSettings_T *settings)
 * @brief Get queue directory path
 * @params settings a SMFSettings_T object
 * @returns queue directory
 */
char *smf_settings_get_queue_dir(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_engine(SMFSettings_T *settings, char *engine)
 * @brief Set engine which should be used
 * @param settings a SMFSettings_T object
 * @param engine engine
 */
void smf_settings_set_engine(SMFSettings_T *settings, char *engine);

/*!
 * @fn char *smf_settings_get_engine(SMFSettings_T *settings)
 * @brief Get configured engine
 * @param settings a SMFSettings_T object
 * @returns engine
 */
char *smf_settings_get_engine(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_modules(SMFSettings_T *settings, char **modules)
 * @brief Set available modules, which will be loaded at runtime.
 * @param settings a SMFSettings_T object
 * @param modules list with available modules
 */
void smf_settings_set_modules(SMFSettings_T *settings, char **modules);

/*!
 * @fn char **smf_settings_get_modules(SMFSettings_T *settings)
 * @brief Get available modules
 * @param settings a SMFSettings_T object
 * @returns modules list
 */
char **smf_settings_get_modules(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_module_fail(SMFSettings_T *settings, int i)
 * @brief Set module_fail setting
 *  1 = proceed and ignore
 *  2 = cancel further processing and return permanet error
 *  3 = cancel further processing and return temporary error (default) 
 * @param settings a SMFSettings_T object
 * @param i module_fail value
 */
void smf_settings_set_module_fail(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_module_fail(SMFSettings_T *settings);
 * @brief Get module_fail setting
 * @param settings a SMFSettings_T object
 * @returns module_fail setting
 */
int smf_settings_get_module_fail(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_nexthop(SMFSettings_T *settings, char *nexthop)
 * @brief Set nexthop setting.
 *  This parameter specifies the final destination, 
 *  after a mail is processed by spmfilter.
 * @param settings a SMFSettings_T object
 * @param nexthop nexthtop string
 */
void smf_settings_set_nexthop(SMFSettings_T *settings, char *nexthop);

/*!
 * @fn char *smf_settings_get_nexthop(SMFSettings_T *settings)
 * @brief Get configured nexthop setting
 * @param settings a SMFSettings_T object
 * @returns nexthop
 */
char *smf_settings_get_nexthop(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_nexthop_fail_code(SMFSettings_T *settings, int i)
 * @brief Set nexthop_fail_code setting
 *  If the delivery to the final destination fails 
 *  for any reason, this code is used as response to 
 *  the sending  MTA (default 451).  
 * @param settings a SMFSettings_T object
 * @param i nexthop_fail_code value
 */
void smf_settings_set_nexthop_fail_code(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_nexthop_fail_code(SMFSettings_T *settings)
 * @brief Get nexthop_fail_code setting
 * @param settings a SMFSettings_T object
 * @returns nexthop_fail_code
 */
int smf_settings_get_nexthop_fail_code(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_nexthop_fail_msg(SMFSettings_T *settings, char *msg)
 * @brief Set nexthop_fail_msg setting.
 *  If the delivery to the final destination fails for any 
 *  reason, this message is used as reponse for the sending
 *  MTA. (default "Requested action aborted: local error in processing").
 * @param settings a SMFSettings_T object
 * @param msg nexthop fail message
 */
void smf_settings_set_nexthop_fail_msg(SMFSettings_T *settings, char *msg);

/*!
 * @fn char *smf_settings_get_nexthop_fail_msg(SMFSettings_T *settings)
 * @brief Get nexthop_fail_msg setting
 * @param settings a SMFSettings_T object
 * @returns nexthop fail message
 */
char *smf_settings_get_nexthop_fail_msg(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_backend(SMFSettings_T *settings, char **backend)
 * @brief Set lookup backend. 
 * @param settings a SMFSettings_T object
 * @param backend backend setting list
 */
void smf_settings_set_backend(SMFSettings_T *settings, char **backend);

/*!
 * @fn char **smf_settings_get_backend(SMFSettings_T *settings)
 * @brief Get backend setting
 * @param settings a SMFSettings_T object
 * @returns backend setting list 
 */
char **smf_settings_get_backend(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_backend_connection(SMFSettings_T *settings, char *conn)
 * @brief Set backend_connection setting.
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
 * @param settings a SMFSettings_T object
 * @param conn backend connection
 */
void smf_settings_set_backend_connection(SMFSettings_T *settings, char *conn);

/*!
 * @fn char *smf_settings_get_backend_connection(SMFSettings_T *settings)
 * @brief Get backend connection setting
 * @param settings a SMFSettings_T object
 * @returns backend connection 
 */
char *smf_settings_get_backend_connection(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_add_header(SMFSettings_T *settings, int i)
 * @brief Define if spmfilter should add it's own header
 * @param settings a SMFSettings_T object
 * @param i add_header value, either 1 (true) or 0 (false)
 */
void smf_settings_set_add_header(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_add_header(SMFSettings_T *settings)
 * @brief Get add_header setting
 * @param settings a SMFSettings_T object
 * @returns add_header value
 */
int smf_settings_get_add_header(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_max_size(SMFSettings_T *settings, unsigned long size)
 * @brief Set max. allowed message size in byte
 * @param settings a SMFSettings_T object
 * @param size max_size setting
 */
void smf_settings_set_max_size(SMFSettings_T *settings, unsigned long size);

/*!
 * @fn unsigned long smf_settings_get_max_size(SMFSettings_T *settings)
 * @brief Get max_size setting in bytes
 * @param settings a SMFSettings_T object
 * @returns max_size value
 */
unsigned long smf_settings_get_max_size(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_tls(SMFSettings_T *settings, SMFTlsOption_T t)
 * @brief Set tls setting
 * @param settings a SMFSettings_T object
 * @param t a SMFTlsOption_T value
 */
void smf_settings_set_tls(SMFSettings_T *settings, SMFTlsOption_T t);

/*! 
 * @fn SMFTlsOption_T smf_settings_get_tls(SMFSettings_T *settings)
 * @brief Get tls setting
 * @param settings a SMFSettings_T object
 * @returns a SMFTlsOption_T value
 */
SMFTlsOption_T smf_settings_get_tls(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_tls_pass(SMFSettings_T *settings, char *pass)
 * @brief Set passphrase for SSL certificate, if needed.
 * @param settings a SMFSettings_T object
 * @param pass tls password
 */
void smf_settings_set_tls_pass(SMFSettings_T *settings, char *pass);

/*!
 * @fn char *smf_settings_get_tls_pass(SMFSettings_T *settings)
 * @brief Get tls_pass setting
 * @param settings a SMFSettings_T object
 * @returns tls password
 */
char *smf_settings_get_tls_pass(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_daemon(SMFSettings_T *settings, int i)
 * @brief Set daemon setting.
 *  Define whether spmfilter should run in 
 *  threaded daemon mode, or not.
 * @param settings a SMFSettings_T object
 * @param i daemon value
 */
void smf_settings_set_daemon(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_daemon(SMFSettings_T *settings)
 * @brief Get daemon setting
 * @params settings a SMFSettings_T object
 * @returns daemon value
 */
int smf_settings_get_daemon(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg)
 * @biref Add smtp return code to list
 * @param settings a SMFSettings_T object
 * @param code smtp code
 * @param msg smtp return message
 */
void smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg);

/*! 
 * @fn char *smf_settings_get_smtp_code(SMFSettings_T *settings, int code)
 * @brief Get smtp return code message of given code
 * @param settings a SMFSettings_T object
 * @param code to look for
 * @returns smtp return message for given code
 */
char *smf_settings_get_smtp_code(SMFSettings_T *settings, int code);

/*!
 * @fn void smf_settings_set_sql_driver(SMFSettings_T *settings, char *driver)
 * @brief Set SQL driver, which should be used.
 *  Possible values are:
 *  - mysql
 *  - postgresql
 *  - sqlite
 * @param settings a SMFSettings_T object
 * @param driver sql_driver value
 */
void smf_settings_set_sql_driver(SMFSettings_T *settings, char *driver);

/*!
 * @fn char *smf_settings_get_sql_driver(SMFSettings_T *settings)
 * @brief Get sql_driver setting
 * @param settings a SMFSettings_T object
 * @returns sql_driver value
 */
char *smf_settings_get_sql_driver(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_name(SMFSettings_T *settings, char *name)
 * @brief Set SQL database name
 * @param settings a SMFSettings_T object
 * @param name database name
 */
void smf_settings_set_sql_name(SMFSettings_T *settings, char *name);

/*!
 * @fn char *smf_settings_get_sql_name(SMFSettings_T *settings)
 * @brief Get SQL database name 
 * @param settings a SMFSettings_T object
 * @returns sql_name value
 */
char *smf_settings_get_sql_name(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_host(SMFSettings_T *settings, char **host)
 * @brief Set SQL host(s)
 * @param settings a SMFSettings_T object
 * @param host sql_host list
 */
void smf_settings_set_sql_host(SMFSettings_T *settings, char **host);

/*!
 * @fn char **smf_settings_get_sql_host(SMFSettings_T *settings)
 * @brief Get SQL host(s)
 * @param settings a SMFSettings_T object
 * @returns sql_host list
 */
char **smf_settings_get_sql_host(SMFSettings_T *settings);

/*!
 * @fn int smf_settings_get_sql_num_hosts(SMFSettings_T *settings)
 * @brief Get number of configured SQL hosts
 * @param settings a SMFSettings_T object
 * @returns sql_num_hosts value
 */
int smf_settings_get_sql_num_hosts(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_port(SMFSettings_T *settings, int port)
 * @brief Set SQL port
 * @param settings a SMFSettings_T object
 * @param port sql port
 */
void smf_settings_set_sql_port(SMFSettings_T *settings, int port);

/*!
 * @fn int smf_settings_get_sql_port(SMFSettings_T *settings)
 * @brief Get SQL port
 * @param settings a SMFSettings_T object
 * @returns sql_port value
 */
int smf_settings_get_sql_port(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_user(SMFSettings_T *settings, char *user)
 * @brief Set SQL username
 * @param settings a SMFSettings_T object
 * @param user sql user value
 */
void smf_settings_set_sql_user(SMFSettings_T *settings, char *user);

/*!
 * @fn char *smf_settings_get_sql_user(SMFSettings_T *settings)
 * @brief Get SQL username
 * @param settings a SMFSettings_T object
 * @returns sql_user value
 */
char *smf_settings_get_sql_user(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_pass(SMFSettings_T *settings, char *pass)
 * @brief Set SQL password
 * @param settings a SMFSettings_T object
 * @param pass sql_pass value
 */
void smf_settings_set_sql_pass(SMFSettings_T *settings, char *pass);

/*!
 * @fn char *smf_settings_get_sql_pass(SMFSettings_T *settings)
 * @brief Get SQL password
 * @param settings a SMFSettings_T object
 * @returns sql_pass value
 */
char *smf_settings_get_sql_pass(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_user_query(SMFSettings_T *settings, char *query)
 * @brief Set sql_user_query setting
 * @param settings a SMFSettings_T object
 * @param query sql user query value
 */
void smf_settings_set_sql_user_query(SMFSettings_T *settings, char *query);

/*!
 * @fn char *smf_settings_get_sql_user_query(SMFSettings_T *settings)
 * @brief Get sql_user_query setting
 * @param settings a SMFSettings_T object
 * @returns sql user query value
 */
char *smf_settings_get_sql_user_query(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_encoding(SMFSettings_T *settings, char *encoding)
 * @brief Set SQL encoding
 * @param settings a SMFSettings_T object
 * @param encoding sql encoding
 */
void smf_settings_set_sql_encoding(SMFSettings_T *settings, char *encoding);

/*!
 * @fn char *smf_settings_get_sql_encoding(SMFSettings_T *settings)
 * @brief Get SQL encoding
 * @param settings a SMFSettings_T object
 * @returns sql encoding
 */
char *smf_settings_get_sql_encoding(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_sql_max_connections(SMFSettings_T *settings, int i)
 * @brief Set max. number of SQL connections
 * @param settings a SMFSettings_T object
 * @param i number of max allowed connections
 */
void smf_settings_set_sql_max_connections(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_sql_max_connections(SMFSettings_T *settings)
 * @brief Get max. number of SQL connections
 * @param settings a SMFSettings_T object
 * @returns number of max. connections
 */
int smf_settings_get_sql_max_connections(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_uri(SMFSettings_T *settings, char *uri)
 * @brief Set LDAP uri
 * @param settings a SMFSettings_T object
 * @param uri ldap uri value
 */
void smf_settings_set_ldap_uri(SMFSettings_T *settings, char *uri);

/*!
 * @fn char *smf_settings_get_ldap_uri(SMFSettings_T *settings)
 * @brief Get LDAP uri
 * @param settings a SMFSettings_T object
 * @returns ldap uri value
 */
char *smf_settings_get_ldap_uri(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_host(SMFSettings_T *settings, char **host)
 * @brief Set LDAP host(s)
 * @param settings a SMFSettings_T object
 * @param host ldap host list
 */
void smf_settings_set_ldap_host(SMFSettings_T *settings, char **host);

/*!
 * @fn char **smf_settings_get_ldap_host(SMFSettings_T *settings)
 * @brief Get LDAP host(s)
 * @param settings a SMFSettings_T object
 * @returns ldap host list
 */
char **smf_settings_get_ldap_host(SMFSettings_T *settings);

/*!
 * @fn int smf_settings_get_ldap_num_hosts(SMFSettings_T *settings)
 * @brief Get number of configured LDAP hosts
 * @param settings a SMFSettings_T object
 * @returns ldap_num_hosts value
 */
int smf_settings_get_ldap_num_hosts(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_port(SMFSettings_T *settings, int port)
 * @brief Set LDAP port
 * @param settings a SMFSettings_T object
 * @param port ldap port value 
 */
void smf_settings_set_ldap_port(SMFSettings_T *settings, int port);

/*!
 * @fn int smf_settings_get_ldap_port(SMFSettings_T *settings)
 * @brief Get LDAP port
 * @param settings a SMFSettings_T object
 * @returns ldap_port value
 */
int smf_settings_get_ldap_port(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_binddn(SMFSettings_T *settings, char *binddn)
 * @brief Set LDAP binddn
 * @param settings a SMFSettings_T object
 * @param binddn ldap binddn value
 */
void smf_settings_set_ldap_binddn(SMFSettings_T *settings, char *binddn);

/*!
 * @fn char *smf_settings_get_ldap_binddn(SMFSettings_T *settings)
 * @brief Get LDAP binddn
 * @param settings a SMFSettings_T object
 * @returns ldap binddn value
 */
char *smf_settings_get_ldap_binddn(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_bindpw(SMFSettings_T *settings, char *bindpw)
 * @brief Set LDAP bind password
 * @param settings a SMFSettings_T object
 * @param bindpw ldap bindpw value
 */
void smf_settings_set_ldap_bindpw(SMFSettings_T *settings, char *bindpw);

/*! 
 * @fn char *smf_settings_get_ldap_bindpw(SMFSettings_T *settings)
 * @brief Get LDAP bind password
 * @param settings a SMFSettings_T object
 * @returns ldap_bindpw value
 */
char *smf_settings_get_ldap_bindpw(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_base(SMFSettings_T *settings, char *base)
 * @brief Set LDAP search base
 * @param settings a SMFSettings_T object
 * @param base ldap base value
 */
void smf_settings_set_ldap_base(SMFSettings_T *settings, char *base);

/*!
 * @fn char *smf_settings_get_ldap_base(SMFSettings_T *settings)
 * @brief Get LDAP search base
 * @param settings a SMFSettings_T object
 * @returns ldap base value
 */
char *smf_settings_get_ldap_base(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_referrals(SMFSettings_T *settings, int i)
 * @brief Set ldap_referrals setting
 * @param settings a SMFSettings_T object
 * @param i ldap referrals value
 */
void smf_settings_set_ldap_referrals(SMFSettings_T *settings, int i);

/*!
 * @fn int smf_settings_get_ldap_referrals(SMFSettings_T *settings)
 * @brief Get ldap_referrals setting
 * @param settings a SMFSettings_T object
 * @returns ldap referrals value
 */
int smf_settings_get_ldap_referrals(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_scope(SMFSettings_T *settings, char *scope)
 * @brief Set LDAP scope 
 *  Possible values are:
 *  - subtree
 *  - onelevel
 *  - base
 * @param settings a SMFSettings_T object
 * @param scope ldap scope value
 */
void smf_settings_set_ldap_scope(SMFSettings_T *settings, char *scope);

/*!
 * @fn char *smf_settings_get_ldap_scope(SMFSettings_T *settings)
 * @brief Get LDAP scope 
 * @param settings a SMFSettings_T object
 * @returns ldap scope
 */
char *smf_settings_get_ldap_scope(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_ldap_user_query(SMFSettings_T *settings, char *query)
 * @brief Set LDAP user query
 * @param settings a SMFSettings_T object
 * @param query ldap user query
 */
void smf_settings_set_ldap_user_query(SMFSettings_T *settings, char *query);

/*!
 * @fn char *smf_settings_get_ldap_user_query(SMFSettings_T *settings)
 * @brief Get LDAP user query
 * @param settings a SMFSettings_T object
 * @returns ldap user_query
 */
char *smf_settings_get_ldap_user_query(SMFSettings_T *settings);

/*!
 * @fn SMFSettingsGroup_T *smf_settings_group_load(SMFSettings_T* settings, char *group_name)
 * @brief Load all settings from specified group
 * @param settings SMFSettings_T object
 * @param group_name wanted group
 * @returns SMFSettingsGroup_T object
 */
SMFSettingsGroup_T *smf_settings_group_load(SMFSettings_T* settings, char *group_name);

/*!
 * @fn char *smf_settings_group_get_value(SMFSettingsGroup_T *group, char *key)
 * @brief Returns the raw value associated with key under the selected group.
 *  Use smf_settings_group_get_string() to retrieve an unescaped UTF-8 string.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_value(SMFSettingsGroup_T *group, char *key);

/*!
 * @fn char *smf_settings_group_get_string(SMFSettingsGroup_T *group, char *key)
 * @brief Returns the string value associated with key under the selected group.
 *  Unlike group_settings_get_value(), this function handles escape
 *  sequences like \s.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get_string(SMFSettingsGroup_T *group, char *key);

/*!
 * @fn int smf_settings_group_get_boolean(SMFSettingsGroup_T *group, char *key)
 * @brief Returns the boolean values associated with key under the selected group as integer.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @returns the value associated with the key as a integer, or 0 if the key was
 * not found or could not be parsed.
 */
int smf_settings_group_get_boolean(SMFSettingsGroup_T *group, char *key);

/*!
 * @fn int smf_settings_group_get_integer(SMFSettingsGroup_T *group,char *key)
 * @brief Returns the value associated with key under the selected group as an integer.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @returns he value associated with the key as an integer, or 0 if the key was not found or could not be parsed.
 */
int smf_settings_group_get_integer(SMFSettingsGroup_T *group,char *key);

/*!
 * @fn double smf_settings_group_get_double(SMFSettingsGroup_T *group, char *key)
 * @brief Returns the value associated with key under the selected group as a double.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @returns the value associated with the key as a double, or 0.0 if the key was not found or could not be parsed.
 */
double smf_settings_group_get_double(SMFSettingsGroup_T *group, char *key);

/*!
 * @fn char **smf_settings_group_get_string_list(SMFSettingsGroup_T *group, char *key, int *length)
 * @brief Returns the values associated with key under the selected group.
 * @param group a SMFSettingsGroup_T object
 * @param key a key
 * @param length return location for the number of returned strings, or NULL
 * @returns a NULL-terminated string array or NULL if the specified key cannot be found.
 */
char **smf_settings_group_get_string_list(SMFSettingsGroup_T *group, char *key, int *length);

/*!
 * @fn void smf_settings_group_free(SMFSettingsGroup_T *group)
 * @brief Free allocated space
 * @param s GroupSettings_T
 */
void smf_settings_group_free(SMFSettingsGroup_T *group);

#endif  /* _SMF_SETTINGS_H */
