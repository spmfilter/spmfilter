#include <stdio.h>

#include "../src/smf_session.h"
#include "../src/smf_trace.h"

#define THIS_MODULE "testmod2"

int load(SMFSession_T *session) {       
    STRACE(TRACE_DEBUG,session->id,"Hello testmod2\n");

    return 0;
}