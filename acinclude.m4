AC_DEFUN([SPMFILTER_CHECK_ESMTP], [
AC_PATH_PROG(esmtpconfig,libesmtp-config)
if test [ -z "$esmtpconfig" ]
then
	AC_MSG_ERROR([libesmtp-config executable not found. Make sure pkg-config is in your path])
else
	AC_MSG_CHECKING([libesmtp])
	ac_esmtp_cflags=`$esmtpconfig --cflags`
	if test -z "$ac_esmtp_cflags"
	then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Unable to loacte libesmtp development files])
	fi
	
	CFLAGS="$CFLAGS $ac_esmtp_cflags"

	ac_esmtp_libs=`${esmtpconfig} --libs`
	if test -z "$ac_esmtp_libs"
	then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Unable to locate libesmtp libaries])
	fi
 
	LDFLAGS="$LDFLAGS $ac_esmtp_libs"
	AC_MSG_RESULT([yes])
fi
])

AC_DEFUN([SPMFILTER_CHECK_GLIB_VERSION], [
	GLIB2_MIN_VERSION=2.14
	AC_MSG_CHECKING([glib is at least version $GLIB2_MIN_VERSION])
	if $PKG_CONFIG --atleast-version $GLIB2_MIN_VERSION glib-2.0
	then
		AC_MSG_RESULT([yes])
	else
		AC_MSG_RESULT([no])
		PKG_CHECK_MODULES([pcre],[libpcre >= 6.0])
		CFLAGS="$CFLAGS $pcre_CFLAGS"
		LDFLAGS="$LDFLAGS $pcre_LIBS"
	fi
])

AC_DEFUN([SPMFILTER_QUEUE_CHECK], [

AC_ARG_WITH(queuedir,
	[  --with-queuedir         set path of spmfilter queue dir (default /var/spool/spmfilter)],
	queuedirname="$withval")

if test [ "$queuedirname" = "" ]; then
	SPMFILTER_QUEUE_DIR=/var/spool/spmfilter
else
	SPMFILTER_QUEUE_DIR=$queuedirname
fi
])


AC_DEFUN([SPMFILTER_LIB_DIR], [
if test `eval echo x$libdir` != xNONE/lib
then
	pkglibdir=`eval echo ${libdir}/${PACKAGE}`
elif test `eval echo x$exec_prefix` != xNONE
then
	pkglibdir=`eval echo ${exec_prefix}/lib/${PACKAGE}`
elif test `eval echo x$prefix` != xNONE
then
	pkglibdir=`eval echo ${prefix}/lib/${PACKAGE}`
else
	pkglibdir=`eval echo /usr/local/lib/${PACKAGE}`
fi
])
