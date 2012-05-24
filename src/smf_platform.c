/* spmfilter - mail filtering framework
 * Copyright (C) 2009-2012 Sebastian Jaekel, Axel Steiner and SpaceNet AG
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

#include "smf_platform.h"

char *smf_build_module_path(const char *libdir, const char *modname) {
    char *path = NULL;
    char *t = NULL;

    if (g_str_has_prefix(modname,"lib")) {
#ifdef __APPLE__
        t = g_strdup_printf("%s.dylib",modname);
#else
        t = g_strdup(modname);
#endif
    } else {
#ifdef __APPLE__
        t = g_strdup_printf("lib%s.dylib", modname);
#else
        t = g_strdup_printf("lib%s", modname);
#endif
    }
    path = (char *)g_module_build_path(libdir,t);
    g_free(t);

    return path;
}
