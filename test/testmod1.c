#include <stdio.h>
#include <string.h>

#include "../src/smf_settings.h"
#include "../src/smf_session.h"
#include "../src/smf_trace.h"

#define THIS_MODULE "testmod1"

#define TEST_MID "<4.2.0.58.20000519003143.00a8d550@pop.example.com>"

int load(SMFSettings_T *settings, SMFSession_T *session) {       
    STRACE(TRACE_DEBUG,session->id,"Hello testmod1\n");
    char *mid = NULL;
    int ret = 0;

    mid = strdup(smf_message_get_message_id(session->envelope->message));
    mid = smf_core_strstrip(mid);

    if (strcmp(mid,TEST_MID) != 0) {
        STRACE(TRACE_ERR,session->id,"expected MID [%s] but got [%s]", TEST_MID, mid);
        ret = -1;
    }
    free(mid);
    return ret;
}