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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <syslog.h>
#include <getopt.h>
#include <errno.h>

#include "spmfilter_config.h"
#include "smf_settings.h"
#include "smf_settings_private.h"
#include "smf_trace.h"
#include "smf_modules.h"
#include "smf_internal.h"

#define THIS_MODULE "spmfilter"

void usage(void) {
    fprintf(stderr,
        "Usage:\n"
        "  spmfilter [OPTION...] - spmfilter options\n\n"
        "Help Options:\n"
        "  -h, --help    Show help options\n\n"
        "Application Options:\n"
        "  -d, --debug   verbose logging\n"
        "  -f, --file    alternate config file\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int ret = 0;
    int option = 0;
    int debug = 0;
    int opt_index = 0;
    char *config_file = NULL;
    SMFSettings_T *settings = NULL;
    struct stat sb;
    
    struct option long_options[] = {
        { "debug", 0, NULL, 'd' },
        { "file", 1, NULL, 'f' },
        { "help", 0, NULL, 'h' },
        { NULL, 0, NULL, 0 }
    };

    while(1) {
        option = getopt_long(argc, argv, "df:h",long_options,&opt_index);
        if (option == EOF)
            break;

        switch(option) {
            case 'h':
                usage();
                break;
            case 'd':
                debug = 1;
                break;
            case 'f':
                config_file = strdup(optarg);
                break;
            default:
                usage();
                break;
        }
    }

    settings = smf_settings_new();
    /* parse config file and fill settings struct */
    if (smf_settings_parse_config(&settings,config_file) != 0) {
        if (config_file != NULL) free(config_file);
        smf_settings_free(settings);
        return -1;
    }

    if (config_file != NULL) free(config_file);

    if (debug == 1)
        smf_settings_set_debug(settings,debug);

    openlog("spmfilter", LOG_PID, smf_settings_get_syslog_facility(settings));

    /* connect to database/ldap server, if necessary */
    if((settings->backend != NULL) && (settings->lookup_persistent == 1)) {
#ifdef HAVE_LDAP
        if (strcmp(settings->backend,"ldap") == 0) {
            if (smf_lookup_ldap_connect(settings) != 0) {
                fprintf(stderr,"spmfilter: unable to establish lookup connection!");
                return -1;
            }
        }
#endif

#ifdef HAVE_ZDB
        if (strcmp(settings->backend,"sql") == 0) {
            if (smf_lookup_sql_connect(settings) != 0) {
                fprintf(stderr,"spmfilter: unable to establish lookup connection!");
                return -1;
            }
        }
#endif  
    }

    /* check queue dir */
    if (stat(settings->queue_dir,&sb) != 0) {
        perror("spmfilter: failed to open queue directory");
        smf_settings_free(settings);
        return -1; 
    }

    if(!S_ISDIR(sb.st_mode)) {
        fprintf(stderr, "spmfilter: [%s] is not a directory",settings->queue_dir);
        smf_settings_free(settings);
        return -1; 
    }
        
    if (access(settings->queue_dir,W_OK) != 0) {
        perror("spmfilter: queue directory is not writeable");
        smf_settings_free(settings);
        return -1; 
    }

    ret = smf_modules_engine_load(settings);

    if((settings->backend != NULL) && (settings->lookup_persistent == 1)) {
#ifdef HAVE_LDAP
        if (strcmp(settings->backend,"ldap") == 0)
            smf_lookup_ldap_disconnect(settings);
#endif

#ifdef HAVE_ZDB
        if (strcmp(settings->backend,"sql") == 0) 
            smf_lookup_sql_disconnect(settings);
#endif  
    }

    /* free all stuff */
    smf_settings_free(settings);

    return ret;
}
