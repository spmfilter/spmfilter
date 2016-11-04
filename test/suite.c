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

#include <check.h>
#include <unistd.h>

extern TCase *core_tcase();
extern TCase *messages_tcase();
extern TCase *modules_tcase();
extern TCase *nexthop_tcase();
extern TCase *ea_tcase();
extern TCase *env_tcase();
extern TCase *dict_tcase();
extern TCase *session_tcase();

static Suite *smf_suite() {
	Suite* s = suite_create("smf unit tests");

	suite_add_tcase(s, core_tcase());
	suite_add_tcase(s, messages_tcase());
	suite_add_tcase(s, modules_tcase());
    suite_add_tcase(s, nexthop_tcase());
    suite_add_tcase(s, ea_tcase());
    suite_add_tcase(s, env_tcase());
    suite_add_tcase(s, dict_tcase());
    suite_add_tcase(s, session_tcase());

	return s;
}

int main(int argc, char * const argv[]) {
	int nfailed;
 	int c;
 	int enable_debug = 0;

 	while ((c = getopt(argc, argv, "d")) != -1) {
		switch (c) {
		case 'd':
			enable_debug = 1;
			break;
		default:
			return 1;
		}
	}

	SRunner* sr = srunner_create(smf_suite());

	if (enable_debug) {
		srunner_set_fork_status(sr, CK_NOFORK);
	}

	srunner_run_all(sr, CK_NORMAL);

	nfailed = srunner_ntests_failed(sr);
	srunner_free(sr);

	return (nfailed == 0) ? 0 : 1;
}
