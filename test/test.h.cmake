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

#ifndef _TEST_H
#define _TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test/samples"

char mime_type_string[] = "text/plain; charset=us-ascii";
char mime_disposition_string[] = "inline";
char mime_encoding_string[] = "quoted-printable";

char id_string[] = "4DF9E5EB.6080300@foo.bar";
char id_string_out[] = "<4DF9E5EB.6080300@foo.bar>";

char test_postface_string[] = "\n";

char test_content_string[] = "\
Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo \
ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis \
dis parturient montes, nascetur ridiculus mus. Donec quam felis, \
ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa \
quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, \
arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. \
Nullam dictum felis eu pede mollis pretium.";

char test_name[] = "John Doe";
char test_email[] = "foo@bar.com";
char test_addr[] = "John Doe <foo@bar.com>";
char test_string[] = "Test string";
char test_prepend[] = "[TEST] Test string";
char test_append[] = "[TEST] Test string [TEST]";

#ifdef __cplusplus
}
#endif

#endif  /* _TEST_H */