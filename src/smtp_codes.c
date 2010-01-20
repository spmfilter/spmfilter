#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spmfilter.h"

#define THIS_MODULE "smtp_codes"

#define ACTIVE 1
#define STARTSIZE 1023

/** Check if given integer is prime
 *
 * \param integer to check
 *
 * \returns 0 on success or -1 in case of error
 */
static unsigned long is_prime(unsigned long val) {
	int i, p, exp, a;

	for (i = 9; i--;) {
		a = (rand() % (val-4)) + 2;
 		p = 1;
		exp = val-1;
		while (exp) {
			if (exp & 1)
				p = (p*a)%val;

			a = (a*a)%val;
			exp >>= 1;
		}

		if (p != 1)
			return 0;
	}

	return 1;
}

/** Find a prime number which is greater
 *  than the given value
 *
 * \param integer value
 *
 * \returns prime number
 */
static int find_prime_greater_than(int val) {
	if (val & 1)
		val+=2;
	else
		val++;

	while (!is_prime(val))
		val+=2;

  return val;
}

/** Rehash SMTP_CODE_TABLE
 *
 * \param SMTP_CODE_TABLE struct
 */
static void rehash(SMTP_CODE_TABLE *ct) {
	long size = ct->size;
	SMTP_CODE *table = ct->table;

	ct->size = find_prime_greater_than(size<<1);
	ct->table = (SMTP_CODE *)calloc(sizeof(SMTP_CODE), ct->size);
	ct->count = 0;

	while(--size >= 0)
		if (table[size].flags == ACTIVE)
			smtp_code_insert(ct, table[size].code, table[size].msg);

	free(table);
}

/** Create a new hash table for all smtp codes
 *
 * \returns new allocated hash table
 */
SMTP_CODE_TABLE *smtp_code_new(void) {
	SMTP_CODE_TABLE *codes = (SMTP_CODE_TABLE *) malloc(sizeof(SMTP_CODE_TABLE));

	codes->table = (SMTP_CODE *) calloc(sizeof(SMTP_CODE), STARTSIZE);
	codes->size = STARTSIZE;
	codes->count = 0;

	return codes;
}

/** Add smtp return code to list
 *
 * \param SMTP_CODE_TABLE struct
 * \param code smtp code
 * \param msg smtp return message
 */
void smtp_code_insert(SMTP_CODE_TABLE *codes, unsigned long code, char *msg) {
	long index, i, step;

	if (codes->size <= codes->count)
		rehash(codes);

	do {
		index = code % codes->size;
		step = (code % (codes->size-2)) + 1;

		for (i = 0; i < codes->size; i++) {
			if (!(codes->table[index].flags & ACTIVE)) {
				codes->table[index].flags |= ACTIVE;
				codes->table[index].msg = malloc(strlen(msg) + 1);
				strcpy(codes->table[index].msg,msg);
				codes->table[index].code = code;
				codes->count++;
				return;
			} else {
				if (codes->table[index].code == code) {
					codes->table[index].msg = msg;
					return;
				}
			}

			index = (index + step) % codes->size;
		}

		rehash(codes);
	} while (1);
}

/** Get smtp return code message of given code
 *
 * \param SMTP_CODE_TABLE struct
 * \param code to look for
 *
 * \returns smtp return message for given code
 */
char *smtp_code_get(SMTP_CODE_TABLE *codes, unsigned long code) {
	if (codes->count) {
		long index, i, step;
		index = code % codes->size;
		step = (code % (codes->size-2)) + 1;

		for (i = 0; i < codes->size; i++) {
			if (codes->table[index].code == code) {
				if (codes->table[index].flags & ACTIVE)
					return codes->table[index].msg;
        		break;
      		} else {
				if (!codes->table[index].msg)
					break;
			}
			index = (index + step) % codes->size;
		}
	}

	return NULL;
}

/** Free smtp codes
 *
 * \param pointer to smtp codes
 */
void smtp_code_free(SMTP_CODE_TABLE *codes) {
	while (--codes->size >= 0)
		if (codes->table[codes->size].flags & ACTIVE)
			if (codes->table[codes->size].msg != NULL)
				free(codes->table[codes->size].msg);
			
	free(codes->table);
	free(codes);
}
