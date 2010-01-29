#include <glib.h>
#include <stdlib.h>


#include "spmfilter.h"

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
