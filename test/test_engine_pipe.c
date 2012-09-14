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

#include "../src/smf_list.h"
#include "../src/smf_settings.h"
#include "../src/smf_settings_private.h"

#define ENGINE "pipe"
#define BUF_SIZE 1024

int main (int argc, char const *argv[]) {
    SMFSettings_T *settings = NULL;
    char *engine = ENGINE;

    if((settings = smf_settings_new()) != NULL) {
        smf_settings_set_debug(settings, 1);

        if (smf_settings_parse_config(&settings,"../../spmfilter/spmfilter.conf.sample") != 0)
            return(-1);

        smf_settings_set_engine(settings,engine); 
    
    } else {
        return -1;
    }

    smf_settings_set_engine(settings,engine);
    printf("ENGINE:[%s]", smf_settings_get_engine(settings));

    load(settings);

    // load funktion aufrufen, settings übergeben
    // ggf. noch ein queue ordner setzen
    // smf_settings liste, dass keine plugins geladen
    // system aufruf (z.b. cat sample | src/spmfilter )
    // return wert 0 ok
  
    return 0;
}