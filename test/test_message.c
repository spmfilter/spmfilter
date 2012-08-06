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
#include <assert.h>
#include <glib.h>
#include <glib/gprintf.h>

#include "../src/smf_message.h"
#include "../src/smf_list.h"
#include "../src/smf_header.h"
#include "../src/smf_part.h"

#include "test.h"

#define TEST_HEADER_NAME "X-Foo"
#define TEST_HEADER_VALUE "foobar"
#define TEST_HEADER "X-Foo: foobar"
#define TEST_CONTENT_TYPE "text/plain; charset=us-ascii"
#define TEST_CONTENT_TRANSFER_ENCODING "multipart/mixed"
#define TEST_MIME_VERSION "1.0"


char test_files[54][10] = {
    "m0001.txt","m0002.txt","m0003.txt","m0004.txt","m0005.txt",
    "m0006.txt","m0007.txt","m0008.txt","m0009.txt","m0010.txt",
    "m0011.txt","m0012.txt","m0013.txt","m0014.txt","m0015.txt",
    "m0016.txt","m0017.txt","m0018.txt","m1001.txt","m1002.txt",
    "m1003.txt","m1004.txt","m1005.txt","m1006.txt","m1007.txt",
    "m1008.txt","m1009.txt","m1010.txt","m1011.txt","m1012.txt",
    "m1013.txt","m1014.txt","m1015.txt","m1016.txt","m2001.txt",
    "m2002.txt","m2003.txt","m2004.txt","m2005.txt","m2006.txt",
    "m2007.txt","m2008.txt","m2009.txt","m2010.txt","m2011.txt",
    "m2012.txt","m2013.txt","m2014.txt","m2015.txt","m2016.txt",
    "m3001.txt","m3002.txt","m3003.txt","m3004.txt"
};

int main (int argc, char const *argv[]) {
    SMFMessage_T *msg = NULL;
    SMFEmailAddress_T *ea = NULL;
    SMFHeader_T *h = NULL;
    SMFList_T *l = NULL;
    SMFListElem_T *elem = NULL;
    char *s = NULL;
    char *s2 = NULL;
    int i, retval;
    char *fname = NULL;
    char *msg_string = NULL;
    FILE *fp = NULL;
    FILE *fp2 = NULL;
    long size = 0;
    SMFPart_T *part = NULL;

    g_printf("Start SMFMessage_T tests...\n");

    g_printf("* testing smf_message_new()...\t\t\t\t\t");
    msg = smf_message_new();
    assert(msg);
    g_printf("passed\n");

    g_printf("* testing smf_message_set_sender()...\t\t\t\t");
    smf_message_set_sender(msg,test_addr);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_sender_string()...\t\t\t");
    s = smf_message_get_sender_string(msg);
    if (strcmp(s,test_addr) !=0 ) {
        g_printf("failed\n");
        return -1;
    } else
        g_printf("passed\n");
    free(s);  

    g_printf("* testing smf_message_get_sender()...\t\t\t\t");
    ea = smf_message_get_sender(msg);
    if (strcmp("John Doe ",smf_email_address_get_name(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    }

    if (strcmp("<foo@bar.com>",smf_email_address_get_email(ea)) != 0) {
        g_printf("failed\n");
        return -1;
    } 
    g_printf("passed\n");

    g_printf("* testing smf_message_generate_message_id()...\t\t\t");
    s = smf_message_generate_message_id();
    assert(s);
    g_printf("passed\n");

    g_printf("* testing smf_message_set_message_id()...\t\t\t");
    smf_message_set_message_id(msg,s);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_message_id()...\t\t\t");
    if (strcmp(s,smf_message_get_message_id(msg)) != 0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);

    g_printf("* testing smf_message_set_header()...\t\t\t\t");
    smf_message_set_header(msg,TEST_HEADER);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_header()...\t\t\t\t");
    h = smf_message_get_header(msg,TEST_HEADER_NAME);
    assert(h);

    if (strcmp(smf_header_get_name(h),TEST_HEADER_NAME)!=0) {
        g_printf("failed\n");
        return -1;
    }

    if (strcmp(smf_header_get_value(h,0),TEST_HEADER_VALUE)!=0) {
        g_printf("failed\n");
        return -1;
    }

    s = smf_header_to_string(h);
    if (strcmp(s,TEST_HEADER)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);

    g_printf("* testing smf_message_add_reciepient()...\t\t\t");
    if (smf_message_add_recipient(msg, test_addr, SMF_EMAIL_ADDRESS_TYPE_TO) != 0) {
        g_printf("failed\n");
        return -1;
    } 
    g_printf("passed\n");

    g_printf("* testing smf_message_get_recipients()...\t\t\t");
    l = smf_message_get_recipients(msg);
    assert(l);

    elem = smf_list_head(l);
    s = smf_email_address_to_string((SMFEmailAddress_T *)smf_list_data(elem));
    if (strcmp(s,test_addr)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s);
   
    g_printf("* testing smf_message_set_content_type()...\t\t\t");
    smf_message_set_content_type(msg,TEST_CONTENT_TYPE);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_content_type()...\t\t\t");
    s = smf_message_get_content_type(msg);
    if (strcmp(s,TEST_CONTENT_TYPE)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_set_content_transfer_encoding()...\t");
    smf_message_set_content_transfer_encoding(msg,TEST_CONTENT_TRANSFER_ENCODING);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_content_transfer_encoding()...\t");
    s = smf_message_get_content_transfer_encoding(msg);
    if (strcmp(s,TEST_CONTENT_TRANSFER_ENCODING)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_set_content_id()...\t\t\t");
    s2 = smf_message_generate_message_id();
    smf_message_set_content_id(msg,s2);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_content_id()...\t\t\t");
    s = smf_message_get_content_id(msg);
    if (strcmp(s,s2)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s2);

    g_printf("* testing smf_message_set_mime_version()...\t\t\t");
    smf_message_set_mime_version(msg,TEST_MIME_VERSION);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_mime_version()...\t\t\t");
    s = smf_message_get_mime_version(msg);
    if (strcmp(s,TEST_MIME_VERSION)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_set_date()...\t\t\t\t");
    if (smf_message_set_date_now(msg)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    
    g_printf("* testing smf_message_get_date()...\t\t\t\t");
    s = smf_message_get_date(msg);
    assert(s);
    g_printf("passed\n");

    g_printf("* testing smf_message_set_subject()...\t\t\t\t");
    smf_message_set_subject(msg,test_string);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_subject()...\t\t\t\t");
    s = smf_message_get_subject(msg);
    if (strcmp(s,test_string)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_prepend_subject()...\t\t\t");
    smf_message_prepend_subject(msg,"[TEST]");
    s = smf_message_get_subject(msg);
    if (strcmp(s,test_prepend)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_append_subject()...\t\t\t");
    smf_message_append_subject(msg,"[TEST]");
    s = smf_message_get_subject(msg);
    if (strcmp(s,test_append)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_set_body()...\t\t\t\t");
    if (smf_message_set_body(msg,test_content_string)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    
    g_printf("* testing smf_message_generate_boundary()...\t\t\t");
    s2 = smf_message_generate_boundary();
    assert(s2);
    g_printf("passed\n");

    g_printf("* testing smf_message_set_boundary()...\t\t\t\t");
    smf_message_set_boundary(msg,s2);
    g_printf("passed\n");

    g_printf("* testing smf_message_get_boundary()...\t\t\t\t");
    s = smf_message_get_boundary(msg);
    if (strcmp(s,s2)!=0) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");
    free(s2);

    g_printf("* testing smf_message_append_part()...\t\t\t\t");
    
    elem = smf_list_head(msg->parts);
    part = (CMimePart_T *)cmime_list_data(elem);
    s = smf_part_get_content(part);

    part = smf_part_new();
    s = g_strdup_printf("%s/redball.png",SAMPLES_DIR);
    if (smf_part_from_file(&part,s,NULL) != 0) {
        g_printf("failed\n");
        return -1;
    }
    free(s);
    if (smf_message_append_part(msg,part) != 0) {
        g_printf("failed\n");
        return -1;
    }
    if (smf_message_get_part_count(msg) != 2) {
        g_printf("failed\n");
        return -1;
    }
    g_printf("passed\n");

    g_printf("* testing smf_message_add_attachment()...\t\t\t");
    s = g_strdup_printf("%s/redball.png",SAMPLES_DIR);
    smf_message_add_attachment(msg,s);

    if (smf_message_get_part_count(msg) != 3) {
        g_printf("failed\n");
        return -1;
    }
    free(s);
    g_printf("passed\n");


    g_printf("* testing smf_message_free()...\t\t\t\t\t");
    smf_message_free(msg);
    g_printf("passed\n");

    g_printf("* testing smf_message_create_skeleton()...\t\t\t");

    
    msg = smf_message_create_skeleton(test_addr,test_addr,test_string);
    if (smf_message_set_body(msg,test_content_string)!=0) {
        g_printf("failed\n");
        return -1;
    }
    
    msg_string = smf_message_to_string(msg);
    assert(msg_string);
    free(msg_string);
  
    smf_message_free(msg);
    g_printf("passed\n");

    g_printf("Start message parsing tests...\n");
    for (i=0; i < 54; i++) {
        g_printf("* checking sample message [%s]...\t\t\t", test_files[i]);
            
        msg = smf_message_new();
        fname = g_strdup_printf("%s/%s",SAMPLES_DIR,test_files[i]);
        retval = smf_message_from_file(&msg,fname,0);
        if (retval != 0)
            return retval;
                
        msg_string = smf_message_to_string(msg);
        
        if ((fp = fopen(fname, "rb")) == NULL) {
            g_printf("failed\n");
            return(-1);
        }
        free(fname);    

        if (fseek(fp, 0, SEEK_END)!=0) {
            g_printf("failed\n");
            return(-1);
        }

        size = ftell(fp);
        rewind(fp); 
        s = (char*) calloc(sizeof(char), size + sizeof(char));
        fread(s, size, 1, fp);
        if(ferror(fp)) {
            g_printf("failed\n");
            return(-1);
        }
            
        fclose(fp);
        s2 = g_strdup_printf("out_%s",test_files[i]);
        fp2 = fopen(s2,"wb");
        fwrite(msg_string,strlen(msg_string),1,fp2);
        fclose(fp2);
        free(s2);
        
        assert(strcmp(msg_string,s)==0);
        free(s);
        free(msg_string);
        smf_message_free(msg);
        printf("passed!\n");
    }

    return 0;
}
