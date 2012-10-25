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

#ifndef _SMF_LOOKUP_PRIVATE_H
#define	_SMF_LOOKUP_PRIVATE_H

#ifdef HAVE_LDAP
#include <ldap.h>
#endif

#ifdef HAVE_ZDB
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#endif

#include "smf_settings.h"
#include "smf_email_address.h"


#ifdef HAVE_LDAP
char *smf_lookup_ldap_get_uri(SMFSettings_T *settings, char *host);
char *smf_lookup_ldap_get_rand_host(SMFSettings_T *settings);
int smf_lookup_ldap_bind(SMFSettings_T *settings);
void smf_lookup_ldap_init_ld(SMFSettings_T **settings,char *uri);
int smf_lookup_ldap_get_scope(SMFSettings_T *settings);
LDAP *smf_lookup_ldap_get_connection(SMFSettings_T *settings);
#endif

#ifdef HAVE_ZDB
char *smf_lookup_sql_get_rand_host(SMFSettings_T *settings);
char *smf_lookup_sql_get_dsn(SMFSettings_T *settings, char *host);
int smf_lookup_sql_start_pool(SMFSettings_T *settings, char *dsn);
void smf_lookup_sql_con_close(Connection_T c);
#endif


#endif	/* _SMF_LOOKUP_PRIVATE_H */

