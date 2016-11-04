/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2016 Axel Steiner and SpaceNet AG
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

#ifndef _TESTDIRS_H
#define _TESTDIRS_H

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test/samples"
#define BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/test"

/* test nexthop for smtp delivery. smtp-sink is a good choice for testing */
#define TEST_NEXTHOP "${TEST_NEXTHOP}"

#ifdef __cplusplus
}
#endif

#endif  /* _TESTDIRS_H */
