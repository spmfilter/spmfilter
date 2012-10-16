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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "../src/smf_list.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"
#include "../src/smf_internal.h"


#define ENGINE "pipe"
#define BUF_SIZE 1024
#define QUEUE_DIR "/tmp"

int load(SMFSettings_T *settings);


int main (int argc, char const *argv[]) {
    SMFSettings_T *settings = NULL;
    char *engine = ENGINE;
    char *queue_dir = QUEUE_DIR;

    if((settings = smf_settings_new()) != NULL) {
        smf_settings_set_debug(settings, 1);
        if (smf_settings_parse_config(&settings,"../../../spmfilter/spmfilter.conf.sample") != 0)
            return(-1);

        smf_settings_set_engine(settings,engine);
    } else {
        return -1;
    }

    smf_settings_set_engine(settings,engine);
    smf_settings_set_queue_dir(settings, queue_dir); 
    
    /* make sure none of the plugins is loaded, therefor we just 
     * clear and recreate settings->modules
     */
    smf_list_free(settings->modules);
    smf_list_new(&settings->modules, smf_internal_string_list_destroy);

    /* call load function */
    if(load(settings) != 0) {
        return -1;
    }
    
    return 0;
}