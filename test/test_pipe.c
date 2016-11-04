/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2016 Axel Steiner, Werner Detter and SpaceNet AG
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
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "../src/smf_list.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_internal.h"
#include "../src/smf_modules.h"

#include "test.h"
#include "test_params.h"

int load(SMFSettings_T *settings);


int main (int argc, char const *argv[]) {
    SMFSettings_T *settings = smf_settings_new();
    int pipefd[2];
    char *fname;

    printf("Start smf_pipe tests...\n");

    fflush(stdout); 

    if (pipe(pipefd)) {
        return -1;
    }

    printf("* preparing pipe engine...\t\t\t");
    smf_settings_set_debug(settings, 1);
    smf_settings_set_queue_dir(settings, BINARY_DIR);
    smf_settings_set_engine(settings, "pipe");
    smf_settings_set_nexthop(settings, "/dev/null");
    
    /* add test modules */
    smf_settings_add_module(settings, BINARY_DIR "/libtestmod1.so");
    smf_settings_add_module(settings, BINARY_DIR "/libtestmod2.so");

    asprintf(&fname, "%s/m0001.txt",SAMPLES_DIR);

    switch (fork()) {
        case -1:
            printf("failed\n");
            return -1;
        case 0:
            close(pipefd[0]); 
            dup2(pipefd[1], 1); 
            close(pipefd[1]);  
            execl("/bin/cat", "cat", fname, (char *)NULL);
            
            break;
        default:
            close(pipefd[1]);  
            dup2(pipefd[0], 0);
            close(pipefd[0]); 
            printf("passed\n");
            printf("* sending test message...\t\t\t\t");
            if (load(settings) != 0) {
                printf("failed\n");
                return -1;
            }

            printf("passed\n");
            break;
    }
    free(fname);
    smf_settings_free(settings);
    return 0;
}
