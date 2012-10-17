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


#include "smf_settings.h"
#include "smf_email_address.h"

char *smf_lookup_ldap_get_uri(SMFSettings_T *settings, char *host);
char *smf_lookup_ldap_get_rand_host(SMFSettings_T *settings);
int smf_lookup_ldap_bind(SMFSettings_T *settings);
void smf_lookup_ldap_init_ld(SMFSettings_T **settings,char *uri);

void smf_lookup_check_user(SMFEmailAddress_T *user);
void smf_lookup_ldap_check_user(char *ldap_uri, SMFEmailAddress_T *user,SMFSettings_T *settings);
void smf_lookup_sql_check_user(SMFSettings_T *settings, SMFEmailAddress_T *user);
int ldap_get_scope(SMFSettings_T *settings);



#endif	/* _SMF_LOOKUP_PRIVATE_H */

