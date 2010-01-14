#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include "spmfilter.h"

void smtp_code_append(SMTP_CODE **codes, int code, char *message) {
	SMTP_CODE *temp, *r;
	temp = *codes;

	if (*codes == NULL) {
		temp = (SMTP_CODE *) malloc(sizeof (SMTP_CODE));
		temp->code = code;
		temp->message = malloc(strlen(message) + 1);
		strcpy(temp->message,message);
		temp->next = NULL;
		*codes = temp;
	} else {
		temp = *codes;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		r = (SMTP_CODE *) malloc(sizeof (SMTP_CODE));
		r->code = code;
		r->message = malloc(strlen(message) + 1);
		strcpy(r->message,message);
		r->next = NULL;
		temp->next = r;
	}
}

char *smtp_code_lookup(SMTP_CODE *codes, int code) {
	while(codes != NULL) {
		if (codes->code == code)
			return codes->message;
		codes = codes->next;
	}

	return NULL;
}

void smtp_codes_free(SMTP_CODE **codes) {
	SMTP_CODE *temp;
	while(codes != NULL) {
		temp = *codes;
		codes = temp->next;

		free(temp->message);
		free(temp);
	}
}

