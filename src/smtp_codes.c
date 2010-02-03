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

#include "spmfilter.h"
#include "smtp_codes.h"

#define THIS_MODULE "smtp_codes"

GHashTable *smtp_codes = NULL;

/** Add smtp return code to list
 *
 * \param code smtp code
 * \param msg smtp return message
 */
void smtp_code_insert(int code, char *msg) {
	g_hash_table_insert(smtp_codes,&code, msg);
}

/** Get smtp return code message of given code
 *
 * \param code to look for
 *
 * \returns smtp return message for given code
 */
char *smtp_code_get(int code) {
	return g_hash_table_lookup(smtp_codes,&code);;
}

/** Create a new hash table for all smtp codes
 *
 * \returns new allocated SmtpCodes_T
 */
SmtpCodes_T *smtp_code_new(void) {
	SmtpCodes_T *codes = (SmtpCodes_T *) malloc(sizeof(SmtpCodes_T));

	smtp_codes = g_hash_table_new((GHashFunc)g_int_hash,(GEqualFunc)g_int_equal);
	codes->get = *smtp_code_get;
	codes->insert = *smtp_code_insert;
	return codes;
}

/** Free smtp codes
 *
 * \param pointer to smtp codes
 */
void smtp_code_free(SmtpCodes_T *codes) {
	free(codes);
	g_hash_table_destroy(smtp_codes);
}
