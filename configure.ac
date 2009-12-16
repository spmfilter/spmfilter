AC_INIT(spmfilter, [0.3.0], ast@treibsand.com, spmfilter)
AC_PREREQ(2.53)
AC_COPYRIGHT(GNU General Public License)
AM_INIT_AUTOMAKE([$PACKAGE_TARNAME], [$PACKAGE_VERSION])
AC_DEFINE(_GNU_SOURCE,'','Defined GNU SOURCE')
AM_ENABLE_SHARED(yes)
AC_PROG_CC
AM_PROG_LIBTOOL


AC_PATH_PROG(pkgconfig,pkg-config)
if test [ -z "$pkgconfig" ] ; then
	AC_MSG_ERROR([pkg-config executable not found. Make sure pkg-config is in your path])
fi

SPMFILTER_CHECK_MATH
SPMFILTER_CHECK_GLIB
SPMFILTER_CHECK_GMIME
SPMFILTER_CHECK_ESMTP
SPMFILTER_LIB_DIR
SPMFILTER_CHECK_ZDB
SPMFILTER_CHECK_LDAP
DEBUG_BUILD

AC_SUBST(GMIME24)
AC_SUBST(PKGCONFIG_GMIME)
AC_SUBST([LIBDIR], [${pkglibdir}])
AC_OUTPUT(Makefile src/Makefile src/spmfilter.h src/libspmfilter.pc man/Makefile)

