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

#define THIS_MODULE "internal"
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void _string_list_destroy(void *data) {
    char *s = (char *)data;
    assert(data);
    free(s);
}

char *_build_module_path(const char *libdir, const char *modname) {
    char *path = NULL;
    char *t = NULL;

    if (strncmp(modname,"lib",3)==0) {
#ifdef __APPLE__
        asprintf(&t,"%s.dylib",modname);
#else
        t = strdup(modname);
#endif
    } else {
#ifdef __APPLE__
        asprintf(&t,"lib%s.dylib", modname);
#else
        asprintf(&t,"lib%s.so",modname);
#endif
    }
    asprintf(&path,"%s/%s",libdir,t);
    free(t);

    return path;
}
