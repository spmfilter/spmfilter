prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=${exec_prefix}/@CMAKE_INSTALL_LIBDIR@/spmfilter
includedir=${prefix}/include/spmfilter

Name: spmfilter
Version: @SMF_VERSION@
Description: Core libraries of spmfilter
URL: http://www.space.net
Libs: -L${libdir} -lsmf
Cflags: -I${includedir}
