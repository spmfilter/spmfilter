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
char mime_version[] = "1.0";
char mime_tranfer_encoding[] = "multipart/mixed";

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
char test_name2[] = "John Doe ";
char test_email[] = "foo@bar.com";
char test_email2[] = "<foo@bar.com>";
char test_addr[] = "John Doe <foo@bar.com>";
char test_addr2[] = "Jane Doe <jane@example.com>";
char test_string[] = "Test string";
char test_string_strip[] = " Test string ";
char test_string_lower[] = "test string";
char test_string_md5[] = "0fd3dbec9730101bff92acc820befc34";
char test_prepend[] = "[TEST] Test string";
char test_append[] = "[TEST] Test string [TEST]";

char test_header_name[] = "X-Foo";
char test_header_value[] = "foobar";
char test_header[] = "X-Foo: foobar";

char test_path[] = "/tmp/test.eml";
char test_auth_user[] = "testusername";
char test_auth_pass[] = "testpassword";
char test_nexthop[] = "localhost:2525";

char test_helo[] = "foo.bar";
char test_ip[] = "127.0.0.1";
char test_response[] = "250 OK message accepted";

char test_config_file[] = "test_settings";
char test_queue_dir[] = ".";
char test_module[] = "clamav";
char test_backend[] = "sql";
char test_backend_conn[] = "failover";
char test_sql_driver[] = "mysql";
char test_sql_name[] = "maildb";
char test_sql_query[] = "select * from test";
char test_encoding[] = "UTF-8";
char test_ldap_base[] = "dc=example,dc=com";
char test_ldap_scope[] = "subtree";
char test_ldap_query[] = "(objectClass=*)";

char test_split[] = "value1;value2;value3";
char test_split_value1[] = "value1";
char test_split_value2[] = "value2";
char test_split_value3[] = "value3";

#ifdef __cplusplus
}
#endif

#endif  /* _TEST_H */