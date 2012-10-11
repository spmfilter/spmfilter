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

#include "spmfilter_config.h"
#include "smf_dict.h"
#include "smf_list.h"

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
    SMFList_T *modules; /**< all configured modules */
    int module_fail; /**< module fail behavior */
    char *nexthop; /**< next smtp hop */
    int nexthop_fail_code; /**< smtp code, when delivery to nexthop fails */
    char *nexthop_fail_msg; /**< smtp return message, when delivery to nexthop fails */
    char *backend; /**< configured lookup backend */
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
    char *lib_dir; /**< user defined directory path for shared libraries */
    char *pid_file; /**< path to pid file */
    char *bind_ip; /**< ip to bind daemon */
    int bind_port; /**< port to bind daemon */
    int listen_backlog; /**< listen queue backlog */
    int foreground; /**< run daemon in foreground */
    char *user; /**< run daemon as user */
    char *group; /** run daemon as group */
    int min_childs; /** number of preforked processed */
    int max_childs; /** maximum number of allowed processes */
    int spare_childs; /** number of spare childs */

    SMFDict_T *smtp_codes; /**< user defined smtp return codes */

    char *sql_driver; /**< sql driver name */
    char *sql_name; /**< sql database name */
    SMFList_T *sql_host; /**< list with sql database hosts */
    int sql_port; /**< sql database port */
    char *sql_user; /**< sql database username */
    char *sql_pass; /**< sql database password */
    char *sql_user_query; /**< sql user query */
    char *sql_encoding; /**< sql encoding */
    int sql_max_connections; /**< max. number of sql conncetions */

    char *ldap_uri; /**< ldap uri */
    SMFList_T *ldap_host; /**< list with ldap hosts */
    int ldap_port; /**< ldap port */
    char *ldap_binddn; /**< ldap bind dn */
    char *ldap_bindpw; /**< ldap bind password */
    char *ldap_base; /**< ldap search base */
    int ldap_referrals; /**< ldap referrals flag */
    char *ldap_scope; /**< ldap search scope */
    char *ldap_user_query; /**< ldap user query */
    void *ldap_connection; /** ldap connection handle LDAP *ld = NULL */

    char *active_lookup_host;   /** storage active lookup host */
    SMFDict_T *groups; /**< custom setting groups */
} SMFSettings_T;


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
 * @fn void smf_settings_add_module(SMFSettings_T *settings, char *module)
 * @brief Add available module to module list, which will be loaded at runtime.
 * @param settings a SMFSettings_T object
 * @param module module name
 * @returns 0 on success or -1 in case of error  
 */
int smf_settings_add_module(SMFSettings_T *settings, char *module);

/*!
 * @fn SMFList_T *smf_settings_get_modules(SMFSettings_T *settings)
 * @brief Get available modules
 * @param settings a SMFSettings_T object
 * @returns modules list
 */
SMFList_T *smf_settings_get_modules(SMFSettings_T *settings);

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
 * @fn void smf_settings_set_backend(SMFSettings_T *settings, char *backend)
 * @brief Set lookup backend. 
 * @param settings a SMFSettings_T object
 * @param backend backend setting list
 */
void smf_settings_set_backend(SMFSettings_T *settings, char *backend);

/*!
 * @fn char *smf_settings_get_backend(SMFSettings_T *settings)
 * @brief Get backend setting
 * @param settings a SMFSettings_T object
 * @returns backend setting 
 */
char *smf_settings_get_backend(SMFSettings_T *settings);

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
 * @fn void smf_settings_set_lib_dir(SMFSettings_T *settings, char *lib_dir)
 * @brief Set path to shared libraries, if needed.
 * @param settings a SMFSettings_T object
 * @param lib_dir path to shared libraries folder
 */
void smf_settings_set_lib_dir(SMFSettings_T *settings, char *lib_dir);

/*!
 * @fn char *smf_settings_get_lib_dir(SMFSettings_T *settings)
 * @brief Get shared library folder
 * @param settings a SMFSettings_T object
 * @returns shared library folder
 */
char *smf_settings_get_lib_dir(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_pid_file(SMFSettings_T *settings, char *pid_file)
 * @brief Set pid file
 * @param settings a SMFSettings_T object
 * @param pid_file char pointer with pid file path 
 */
void smf_settings_set_pid_file(SMFSettings_T *settings, char *pid_file);

/*!
 * @fn char *smf_settings_get_pid_file(SMFSettings_T *settings)
 * @brief Get current pid file
 * @param settings a SMFSettings_T object
 * @returns char pointer with pid file path
 */
char *smf_settings_get_pid_file(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_bind_ip(SMFSettings_T *settings, char *ip)
 * @brief Set bind ip 
 * @param settings a SMFSettings_T object
 * @param ip char pointer with ip 
 */
void smf_settings_set_bind_ip(SMFSettings_T *settings, char *ip);

/*!
 * @fn char *smf_settings_get_bind_ip(SMFSettings_T *settings)
 * @brief Get current bind ip
 * @param settings a SMFSettings_T object
 * @returns char pointer with ip
 */
char *smf_settings_get_bind_ip(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_bind_port(SMFSettings_T *settings, int port)
 * @brief Set bind port 
 * @param settings a SMFSettings_T object
 * @param port port number
 */
void smf_settings_set_bind_port(SMFSettings_T *settings, int port);

/*!
 * @fn int smf_settings_get_bind_port(SMFSettings_T *settings)
 * @brief Get current bind port
 * @param settings a SMFSettings_T object
 * @returns port number
 */
int smf_settings_get_bind_port(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_listen_backlog(SMFSettings_T *settings, int backlog)
 * @brief Set listen backlog 
 * @param settings a SMFSettings_T object
 * @param backlog max. number of listen backlog queue
 */
void smf_settings_set_listen_backlog(SMFSettings_T *settings, int backlog);

/*!
 * @fn int smf_settings_get_listen_backlog(SMFSettings_T *settings)
 * @brief Get listen backlog number
 * @param settings a SMFSettings_T object
 * @returns listen backlog number
 */
int smf_settings_get_listen_backlog(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_foreground(SMFSettings_T *settings, int foreground)
 * @brief Set foreground config option
 * @param settings a SMFSettings_T object
 * @param foreground foreground config options (1 = true, 0 = false)
 */
void smf_settings_set_foreground(SMFSettings_T *settings, int foreground);

/*!
 * @fn int smf_settings_get_foreground(SMFSettings_T *settings)
 * @brief Get foreground setting
 * @param settings a SMFSettings_T object
 * @returns foreground setting
 */
int smf_settings_get_foreground(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_user(SMFSettings_T *settings, char *user)
 * @brief Set effective user 
 * @param settings a SMFSettings_T object
 * @param user effective username
 */
void smf_settings_set_user(SMFSettings_T *settings, char *user);

/*!
 * @fn char *smf_settings_get_user(SMFSettings_T *settings)
 * @brief Get effective user
 * @param settings a SMFSettings_T object
 * @returns char pointer with username
 */
char *smf_settings_get_user(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_group(SMFSettings_T *settings, char *group)
 * @brief Set effective group
 * @param settings a SMFSettings_T object
 * @param user effective groupname
 */
void smf_settings_set_group(SMFSettings_T *settings, char *group);

/*!
 * @fn char *smf_settings_get_group(SMFSettings_T *settings)
 * @brief Get effective group
 * @param settings a SMFSettings_T object
 * @returns char pointer with groupname
 */
char *smf_settings_get_group(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_min_childs(SMFSettings_T *settings, int min_childs)
 * @brief Set the number of preforked processes
 * @param settings a SMFSettings_T object
 * @param min_childs number of processes
 */
void smf_settings_set_min_childs(SMFSettings_T *settings, int min_childs);

/*!
 * @fn int smf_settings_get_min_childs(SMFSettings_T *settings)
 * @brief Get number preforked processes
 * @param settings a SMFSettings_T object
 * @returns number of processes
 */
int smf_settings_get_min_childs(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_max_childs(SMFSettings_T *settings, int max_childs)
 * @brief Set the number of maximal allowed processes
 * @param settings a SMFSettings_T object
 * @param max_childs number of processes
 */
void smf_settings_set_max_childs(SMFSettings_T *settings, int max_childs);

/*!
 * @fn int smf_settings_get_max_childs(SMFSettings_T *settings)
 * @brief Get number of maxmimal allowed processes
 * @param settings a SMFSettings_T object
 * @returns number of processes
 */
int smf_settings_get_max_childs(SMFSettings_T *settings);

/*!
 * @fn void smf_settings_set_spare_childs(SMFSettings_T *settings, int spare_childs)
 * @brief Set the number of spare child processes
 * @param settings a SMFSettings_T object
 * @param spare_childs number of spare processes
 */
void smf_settings_set_spare_childs(SMFSettings_T *settings, int spare_childs);

/*!
 * @fn int smf_settings_get_spare_childs(SMFSettings_T *settings)
 * @brief Get number of spare processes
 * @param settings a SMFSettings_T object
 * @returns number of spare processes
 */
int smf_settings_get_spare_childs(SMFSettings_T *settings);

/*!
 * @fn int smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg)
 * @biref Add smtp return code to list
 * @param settings a SMFSettings_T object
 * @param code smtp code
 * @param msg smtp return message
 * @returns 0 on success or -1 in case of error
 */
int smf_settings_set_smtp_code(SMFSettings_T *settings, int code, char *msg);

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
 * @fn int smf_settings_add_sql_host(SMFSettings_T *settings, char *host)
 * @brief Set SQL host(s)
 * @param settings a SMFSettings_T object
 * @param host sql_host list
 * @returns 0 on success or -1 in case of error  
 */
int smf_settings_add_sql_host(SMFSettings_T *settings, char *host);

/*!
 * @fn SMFList_T *smf_settings_get_sql_hosts(SMFSettings_T *settings)
 * @brief Get SQL host(s)
 * @param settings a SMFSettings_T object
 * @returns sql_host list
 */
SMFList_T *smf_settings_get_sql_hosts(SMFSettings_T *settings);

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
 * @fn int smf_settings_add_ldap_host(SMFSettings_T *settings, char *host)
 * @brief Set LDAP host(s)
 * @param settings a SMFSettings_T object
 * @param host ldap host list
 * @returns 0 on success or -1 in case of error  
 */
int smf_settings_add_ldap_host(SMFSettings_T *settings, char *host);

/*!
 * @fn SMFList_T *smf_settings_get_ldap_hosts(SMFSettings_T *settings)
 * @brief Get LDAP host(s)
 * @param settings a SMFSettings_T object
 * @returns ldap host list
 */
SMFList_T *smf_settings_get_ldap_hosts(SMFSettings_T *settings);

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
 * @fn char *smf_settings_set_active_lookup_host(SMFSettings_T *settings)
 * @brief Set current active lookup host
 * @param settings a SMFSettings_T object
 */
void smf_settings_set_active_lookup_host(SMFSettings_T *settings, char *host);

/*!
 * @fn char *smf_settings_get_active_lookup_host(SMFSettings_T *settings)
 * @brief Get current active lookup host
 * @param settings a SMFSettings_T object
 * @param host char pointer with hostname/IP 
 * @returns char pointer with active lookup host
 */
char *smf_settings_get_active_lookup_host(SMFSettings_T *settings);

/*!
 * @fn char *smf_settings_group_get(SMFSettingsGroup_T *group, char *group_name, char *key)
 * @brief Returns the raw value associated with key under the selected group.
 * @param group a SMFSettings_T object
 * @param group_name name of the settings section
 * @param key a key
 * @returns a newly allocated string or NULL if the specified key cannot be found.
 */
char *smf_settings_group_get(SMFSettings_T *settings, char *group_name, char *key);

/*!
 * @fn int smf_settings_group_get_integer(SMFSettings_T *settings, char *group_name, char *key)
 * @brief Returns the value associated with key under the selected group as an integer.
 * @param group a SMFSettings_T object
 * @param group_name name of the settings section
 * @param key a key
 * @returns he value associated with the key as an integer, or 0 if the key was not found or could not be parsed.
 */
int smf_settings_group_get_integer(SMFSettings_T *settings, char *group_name, char *key);

/*!
 * @fn int smf_settings_group_get_boolean(SMFSettings_T *settings, char *group_name, char *key)
 * @brief Returns the boolean values associated with key under the selected group as integer.
 * @param group a SMFSettings_T object
 * @param group_name name of the settings section
 * @param key a key
 * @returns the value associated with the key as a integer, 1 if true, 0 if false
 */
int smf_settings_group_get_boolean(SMFSettings_T *settings, char *group_name, char *key);

/*!
 * @fn SMFList_T *smf_settings_group_get_list(SMFSettings_T *settings, char *group_name, char *key)
 * @brief Returns the values associated with key under the selected group.
 * @param group a SMFSettings_T object
 * @param group_name name of the settings section
 * @param key a key
 * @returns a newly allocated SMFList_T object or NULL on failure
 */
SMFList_T *smf_settings_group_get_list(SMFSettings_T *settings, char *group_name, char *key);

#endif  /* _SMF_SETTINGS_H */
