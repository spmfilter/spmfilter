/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Axel Steiner, Werner Detter and SpaceNet AG
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

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "test.h"
#include "../src/smf_internal.h"

#define CRLF "\r\n"
#define LF "\n"
#define CR "\r"


int remove_testfile(char *absolute_path_to_file) {
    if((remove(absolute_path_to_file)) < 0) {
      fprintf(stderr, "error deleting test file %s", absolute_path_to_file);
      return -1;
    }
    return 0;
}

int main (int argc, char const *argv[]) {
	

	int fd;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	ssize_t br;
	char *out_s = "this is a test message";
	char buf[MAXLINE];
	void *rl = NULL;
	char *out_f = NULL;
    char *res = NULL;

	asprintf(&out_f, "%s/%s", SAMPLES_DIR, "test_smf_internal.txt");

	printf("Start smf_internal tests...\n");

	printf("* testing smf_internal_build_module_path()...\t\t\t");
	res = smf_internal_build_module_path(test_internal_libdir, test_internal_modname);
	if(strcmp(res,test_internal_build_module_path_res)!=0) {
	   printf("failed\n");
        return -1;
	} 
	printf("passed\n");
    free(res);


	printf("* testing smf_internal_strip_email_addr()...\t\t\t");
	res = smf_internal_strip_email_addr(test_internal_strip_email_addr);
	if(strcmp(res,test_internal_strip_email_addr_res)!=0) {
	   printf("failed\n");
        return -1;
	} 
	printf("passed\n");
    free(res);


	printf("* testing smf_internal_writen() \t\t\t\t");
	fd = open(out_f, O_WRONLY | O_CREAT | O_TRUNC, mode);
	smf_internal_writen(fd,out_s,strlen(out_s));
	printf("passed\n");

	printf("* testing smf_internal_readline() \t\t\t\t");
	while((br = smf_internal_readline(fd,buf,MAXLINE,&rl)) > 0) {
		if (strcmp(buf,out_s)!=0) {
			printf("failed\n");
        		return -1;
		}
	}
	printf("passed\n");
	lseek(fd,0L,SEEK_SET);

	printf("* testing smf_internal_readn()\t\t\t\t\t");
	smf_internal_readn(fd,buf,strlen(out_s));
	while((br = smf_internal_readn(fd,buf,strlen(out_s)) > 0)) {
		if (strcmp(buf,out_s)!=0) {
			printf("failed\n");
        		return -1;
		}
	}
	printf("passed\n");


    printf("* testing smf_internal_determine_linebreak with CRLF() \t\t");
    if(strcmp(smf_internal_determine_linebreak(test_internal_determine_linebreak_CRLF),CRLF) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_internal_determine_linebreak with CR() \t\t");
    if(strcmp(smf_internal_determine_linebreak(test_internal_determine_linebreak_CR),CR) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");

    printf("* testing smf_internal_determine_linebreak with LF() \t\t");
    if(strcmp(smf_internal_determine_linebreak(test_internal_determine_linebreak_LF),LF) != 0) {
        printf("failed\n");
        return -1;
    }
    printf("passed\n");


    close(fd);
    remove_testfile(out_f);

    if(out_f != NULL)
        free(out_f);

    if(rl != NULL)
        free(rl);


	return 0;
}

