prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=${exec_prefix}/lib/spmfilter
includedir=${prefix}/include

Name: spmfilter
Version: @SMF_VERSION@
Description: Core libraries of spmfilter
URL: http://www.space.net
Libs: -L${libdir} -lsmf
Cflags: -I${includedir}
