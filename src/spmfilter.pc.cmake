prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@CMAKE_INSTALL_PREFIX@
libdir=@CMAKE_INSTALL_FULL_LIBDIR@/spmfilter
includedir=${prefix}/include
smf_includedir=${includedir}/spmfilter

Name: spmfilter
Version: @SMF_VERSION@
Description: Core libraries of spmfilter
URL: http://www.space.net
Libs: -L${libdir} -lsmf
Cflags: -I${includedir} -I${smf_includedir}
