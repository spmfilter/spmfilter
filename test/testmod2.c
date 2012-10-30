#include <stdio.h>

#include "../src/smf_session.h"

#define THIS_MODULE "testmod2"

int load(SMFSession_T *session) {       
    printf("Hello testmod2\n");

    return 0;
}