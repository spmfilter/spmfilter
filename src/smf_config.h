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

#ifndef SPMFILTER_CONFIG_H
#define SPMFILTER_CONFIG_H

/* spmfilter version */
#define SMF_VERSION 3000
#define SMF_MAJOR_VERSION 0
#define SMF_MINOR_VERSION 3
#define SMF_MICRO_VERSION 0

/* path for spmfilter plugins */
#define LIB_DIR "/usr/lib/spmfilter"

/* gmime */
#define HAVE_GMIME24

/* ldap */
#define HAVE_LDAP

/* zdb */
#define HAVE_ZDB

/* db4 */
#define HAVE_DB4

/* pcre */
#define HAVE_PCRE

#endif /* SPMFILTER_CONFIG_H */
