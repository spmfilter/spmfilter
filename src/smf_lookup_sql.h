/* spmfilter - mail filtering framework
 * Copyright (C) 2012 Axel Steiner and SpaceNet AG
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

#ifndef _SMF_LOOKUP_SQL_H
#define _SMF_LOOKUP_SQL_H

#ifdef HAVE_ZDB
#include <zdb.h>

typedef struct {
    ConnectionPool_T pool;
    URL_T url;
} SMFSQLConnection_T;

char *smf_lookup_sql_get_rand_host(SMFSettings_T *settings);
char *smf_lookup_sql_get_dsn(SMFSettings_T *settings, char *host);
int smf_lookup_sql_start_pool(SMFSettings_T *settings, char *dsn);
void smf_lookup_sql_con_close(Connection_T c);
Connection_T smf_lookup_sql_get_connection(ConnectionPool_T pool);
#endif

#endif  /* _SMF_LOOKUP_SQL_H */
