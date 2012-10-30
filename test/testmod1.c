#include <stdio.h>

#include "../src/smf_session.h"

#define THIS_MODULE "testmod1"

int load(SMFSession_T *session) {       
    printf("Hello testmod1\n");

    return 0;
}