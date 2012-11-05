#include <stdio.h>

#include "../src/smf_session.h"
#include "../src/smf_trace.h"

#define THIS_MODULE "testmod1"

int load(SMFSession_T *session) {       
    STRACE(TRACE_DEBUG,session->id,"Hello testmod1\n");

    return 0;
}