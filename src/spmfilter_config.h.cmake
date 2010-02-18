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

#ifndef _SPMFILTER_CONFIG_H
#define _SPMFILTER_CONFIG_H

/* spmfilter version */
#define SMF_VERSION ${SMF_VERSION_NUMBER}
#define SMF_MAJOR_VERSION ${SMF_MAJOR_VERSION}
#define SMF_MINOR_VERSION ${SMF_MINOR_VERSION}
#define SMF_MICRO_VERSION ${SMF_MICRO_VERSION}

/* path for spmfilter plugins */
#define LIB_DIR "${SMF_LIB_DIR}"

/* gmime */
#cmakedefine HAVE_GMIME24

/* ldap */
#cmakedefine HAVE_LDAP

/* zdb */
#cmakedefine HAVE_ZDB

/* db4 */
#cmakedefine HAVE_DB4

/* pcre */
#cmakedefine HAVE_PCRE

#endif /* _SPMFILTER_CONFIG_H */
