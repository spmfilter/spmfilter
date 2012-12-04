#include <stdio.h>

#include "../src/smf_settings.h"
#include "../src/smf_session.h"
#include "../src/smf_trace.h"

#define THIS_MODULE "testmod1"

int load(SMFSettings_T *settings, SMFSession_T *session) {       
    STRACE(TRACE_DEBUG,session->id,"Hello testmod1\n");

    return 0;
}