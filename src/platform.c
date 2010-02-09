/*
 * file: platform.c
 * desc: contains platform dependent code
 * auth: Sebastian Jaekel <sj@space.net>
 */

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

gchar *smf_build_module_path(const char *libdir, const char *modname) {
	if (g_str_has_prefix(modname,"lib")) {
#ifdef __APPLE__
		return g_module_build_path(libdir,g_strdup_printf("%s.dylib",modname));
#else
		return g_module_build_path(libdir, modname);
#endif
	} else {
#ifdef __APPLE__
		return g_module_build_path(libdir,g_strdup_printf("lib%s.dylib", modname));
#else
		return g_module_build_path(libdir,g_strdup_printf("lib%s", modname));
#endif
	}

	return(NULL); /* should never be reached */
}
