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

#include <glib.h>
#include <stdlib.h>

#include "smf_smtp_codes.h"

#define THIS_MODULE "smtp_codes"

GHashTable *smtp_codes = NULL;

/** Add smtp return code to list
 *
 * \param code smtp code
 * \param msg smtp return message
 */
void smf_smtp_codes_insert(int code, char *msg) {
	if (smtp_codes == NULL)
		smtp_codes = (void *)g_hash_table_new((GHashFunc)g_int_hash,(GEqualFunc)g_int_equal);
	g_hash_table_insert(smtp_codes,&code, msg);
}

/** Get smtp return code message of given code
 *
 * \param code to look for
 *
 * \returns smtp return message for given code
 */
char *smf_smtp_codes_get(int code) {
	if (smtp_codes != NULL)
		return g_hash_table_lookup(smtp_codes,&code);
	else
		return NULL;
}

/** Free smtp codes */
void smf_smtp_codes_free(void) {
	g_hash_table_destroy(smtp_codes);
}
