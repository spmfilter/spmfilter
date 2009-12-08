AC_DEFUN([SPMFILTER_CHECK_GLIB], [
	AC_MSG_CHECKING([GLib headers])
	ac_glib_cflags=`${pkgconfig} --cflags glib-2.0`
	if test -z "$ac_glib_cflags"
	then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Unable to locate glib development files])
	fi
 
	CFLAGS="$CFLAGS $ac_glib_cflags"
	AC_MSG_RESULT([yes])
	AC_MSG_CHECKING([Glib libraries])
	ac_glib_libs=`${pkgconfig} --libs glib-2.0`
	if test -z "$ac_glib_libs"
	then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Unable to locate glib libaries])
	fi

	LDFLAGS="$LDFLAGS $ac_glib_libs"
	AC_MSG_RESULT([yes])
	
	GLIB2_MIN_VERSION=2.14
	AC_MSG_CHECKING([Glib is at least version $GLIB2_MIN_VERSION])
	if $pkgconfig --atleast-version $GLIB2_MIN_VERSION glib-2.0
	then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
		AC_MSG_CHECKING([pcre headers])
		ac_pcre_flags=`${pkgconfig} --cflags libpcre`
		if test -z "$ac_pcre_cflags"
		then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to locate pcre development files])
		fi
		
		CFLAGS="$CFLAGS $ac_pcre_CFLAGS"
		AC_MSG_CHECKING([pcre libraries])
		ac_pcre_libs=`${pkgconfig} --libs libpcre`
		if test -z "$ac_pcre_libs"
		then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to locate pcre libaries])
		fi
		AC_DEFINE([HAVE_PCRE])
		LDFLAGS="$LDFLAGS $ac_pcre_LIBS"
	fi
])

AC_DEFUN([SPMFILTER_CHECK_ESMTP], [
	AC_PATH_PROG(esmtpconfig,libesmtp-config)
	if test [ -z "$esmtpconfig" ] ; then
		AC_MSG_ERROR([libesmtp-config executable not found. Make sure libesmtp-config is in your path])
	else
		AC_MSG_CHECKING([libesmtp])
		ac_esmtp_cflags=`$esmtpconfig --cflags`
		if test -z "$ac_esmtp_cflags" ; then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to loacte libesmtp development files])
		fi
	
		CFLAGS="$CFLAGS $ac_esmtp_cflags"

		ac_esmtp_libs=`${esmtpconfig} --libs`
		if test -z "$ac_esmtp_libs" ; then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to locate libesmtp libaries])
		fi
 
		LDFLAGS="$LDFLAGS $ac_esmtp_libs"
		AC_MSG_RESULT([yes])
	fi
])

AC_DEFUN([SPMFILTER_CHECK_GMIME], [
	AC_MSG_CHECKING([GMime24 headers])
	ac_gmime_cflags=`${pkgconfig} --cflags gmime-2.4`
	if test -z "$ac_gmime_cflags" ; then
		AC_MSG_RESULT([no])
	else
		CFLAGS="$CFLAGS $ac_gmime_cflags"
		AC_MSG_RESULT([yes])
	fi
	
	
	AC_MSG_CHECKING([GMime24 libraries])
	ac_gmime_libs=`${pkgconfig} --libs gmime-2.4`
	if test -z "$ac_gmime_libs" ; then
		AC_MSG_RESULT([no])
		ac_gmime24="no"
	else
		LDFLAGS="$LDFLAGS $ac_gmime_libs"
		AC_MSG_RESULT([yes])
		ac_gmime24="yes"
		AC_DEFINE([HAVE_GMIME24])
	fi
	
	if test [ "x$ac_gmime24" != "xyes" ] ; then
		AC_MSG_CHECKING([GMime headers])
		ac_gmime_cflags=`${pkgconfig} --cflags gmime-2.0`
		if test -z "$ac_gmime_cflags" ; then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to locate gmime development files])
		else
			CFLAGS="$CFLAGS $ac_gmime_cflags"
			AC_MSG_RESULT([yes])
		fi

		AC_MSG_CHECKING([GMime libraries])
		ac_gmime_libs=`${pkgconfig} --libs gmime-2.0`
		if test -z "$ac_gmime_libs" ; then
			AC_MSG_RESULT([no])
			AC_MSG_ERROR([Unable to locate gmime libaries])
		else
			LDFLAGS="$LDFLAGS $ac_gmime_libs"
			AC_MSG_RESULT([yes])
		fi
	fi
])

AC_DEFUN([SPMFILTER_CHECK_LDAP], [
	AC_ARG_WITH(ldap,
		[  --with-ldap=PATH        path to ldap base directory (e.g. /usr/local or /usr)],
		[lookforldap="$withval"],[lookforldap="no"])
	
	if test [ "x$lookforldap" != "xno" ] ; then
		if test [ "x$withval" = "x" ] ; then
			CFLAGS="$CFLAGS -I${prefix}/include"
		else
			CFLAGS="$CFLAGS -I${lookforldap}/include"
		fi
		
		AC_CHECK_HEADERS(ldap.h, [] ,AC_MSG_ERROR([Unable to locate OpenLDAP development files]))
		AC_CHECK_LIB(ldap, ldap_initialize, [], AC_MSG_ERROR([Unable to locate OpenLDAP libraries]))
		
		AC_DEFINE([HAVE_LDAP])
	fi
])


AC_DEFUN([SPMFILTER_CHECK_ZDB], [
	AC_ARG_WITH(zdb,
	[  --with-zdb=PATH         path to libzdb base directory (e.g. /usr/local or /usr)],
	[lookforzdb="$withval"],[lookforzdb="no"])

	
	if test [ "x$lookforzdb" != "xno" ] ; then
		if test [ "x$withval" = "x" ] ; then
			CFLAGS="$CFLAGS -I${prefix}/include/zdb"
		else
			CFLAGS="$CFLAGS -I${lookforzdb}/include/zdb"
		fi
	
		AC_CHECK_HEADERS([URL.h ResultSet.h PreparedStatement.h Connection.h ConnectionPool.h SQLException.h],
			[ZDBLIB="-lzdb"], 
			[ZDBLIB="failed"],
			[[
#include <URL.h>
#include <ResultSet.h>
#include <PreparedStatement.h>
#include <Connection.h>
#include <ConnectionPool.h>
#include <SQLException.h>       
			]])

			if test [ "x$ZDBLIB" = "xfailed" ]; then
				AC_MSG_ERROR([Could not find ZDB library.])
			else
				LDFLAGS="$LDFLAGS $ZDBLIB"
				AC_DEFINE([HAVE_ZDB])
			fi
	fi
	
])

AC_DEFUN([SPMFILTER_CHECK_MATH], [dnl
	AC_CHECK_HEADERS([math.h],[MATHLIB="-lm"], [MATHLIB="failed"])
	if test [ "x$MATHLIB" = "xfailed" ]; then
		AC_MSG_ERROR([Could not find MATH library.])
	else
		LDFLAGS="$LDFLAGS $MATHLIB"
	fi
])


AC_DEFUN([SPMFILTER_LIB_DIR], [
	if test `eval echo x$libdir` != xNONE/lib ; then
		pkglibdir=`eval echo ${libdir}/${PACKAGE}`
	elif test `eval echo x$exec_prefix` != xNONE ; then
		pkglibdir=`eval echo ${exec_prefix}/lib/${PACKAGE}`
	elif test `eval echo x$prefix` != xNONE ; then
		pkglibdir=`eval echo ${prefix}/lib/${PACKAGE}`
	else
		pkglibdir=`eval echo /usr/local/lib/${PACKAGE}`
	fi
])
