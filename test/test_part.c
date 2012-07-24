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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_part.h"

#include "test.h"

int main (int argc, char const *argv[]) {
    SMFPart_T *part = NULL;
    char *s = NULL;
    char *out = NULL;
    char *out2 = NULL;
    FILE *fp = NULL;
    size_t size;

    g_printf("Start SMFPart_T tests...\n");

    g_printf("* testing smf_part_new()...\t\t\t\t\t");
    part = smf_part_new();
    assert(part);
    g_printf("passed\n");

    g_printf("* testing smf_part_set_content_type()...\t\t\t");
    smf_part_set_content_type(part,mime_type_string);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_content_type()...\t\t\t");
    s = smf_part_get_content_type(part);
    if (strcmp(s,mime_type_string) !=0 ) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_set_content_disposition()...\t\t\t");
    smf_part_set_content_disposition(part,mime_disposition_string);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_content_disposition()...\t\t\t");
    s = smf_part_get_content_disposition(part);
    if (strcmp(s,mime_disposition_string) !=0 ) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_set_content_transfer_encoding()...\t\t");
    smf_part_set_content_transfer_encoding(part,mime_encoding_string);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_content_transfer_encoding()...\t\t");
    s = smf_part_get_content_transfer_encoding(part);
    if (strcmp(s,mime_encoding_string) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_set_content_id()...\t\t\t\t");
    smf_part_set_content_id(part,id_string_out);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_content_id()...\t\t\t\t");
    s = smf_part_get_content_id(part);
    if (strcmp(s,id_string_out) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_set_content()...\t\t\t\t");
    smf_part_set_content(part,test_content_string);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_content()...\t\t\t\t");
    s = smf_part_get_content(part);
    if (strcmp(s,test_content_string) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_set_postface()...\t\t\t\t");
    smf_part_set_postface(part, test_postface_string);
    g_printf("passed\n");

    g_printf("* testing smf_part_get_postface()...\t\t\t\t");
    s = smf_part_get_postface(part);
    if (strcmp(s,test_postface_string) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_to_string()...\t\t\t\t");
    out = smf_part_to_string(part,NULL);
    assert(s);
    g_printf("passed\n");

    g_printf("* testing smf_part_free()...\t\t\t\t\t");
    smf_part_free(part);
    g_printf("passed\n");

    g_printf("* testing smf_part_from_string()...\t\t\t\t");
    part = smf_part_new();
    if (smf_part_from_string(&part,out) !=0 ) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_part_to_string()...\t\t\t\t");
    out2 = smf_part_to_string(part,NULL);
    if (strcmp(out,out2) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(out);
    free(out2);
    smf_part_free(part);

    g_printf("* testing smf_part_from_file()...\t\t\t\t");
    part = smf_part_new();
    s = g_strdup_printf("%s/c0001.txt",SAMPLES_DIR);
    smf_part_from_file(&part,s,NULL);
    free(s);

    s = g_strdup_printf("%s/c0001_part.txt",SAMPLES_DIR);
    fp = fopen(s, "rb");
    if (fp == NULL) {
        g_printf("error opening file");
        return -1;
    }
    free(s);
    
    fseek(fp, 0, SEEK_END);
    
    size = ftell(fp);
    rewind(fp); 
    s = (char*) calloc(sizeof(char), size + sizeof(char));
    fread(s, size, 1, fp);
    fclose(fp);

    out = smf_part_to_string(part,NULL);

    if (strcmp(out,s) !=0 ) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(out);
    free(s);
        
    smf_part_free(part);


    return 0;
}
