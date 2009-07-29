AC_DEFUN([SPMFILTER_SET_VERSION], [
	spmfilter_version=`cat VERSION`
	AC_DEFINE_UNQUOTED([SPMFILTER_VERSION], "$spmfilter_version", [Includes the micro version])
])

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

AC_DEFUN([SPMFILTER_CHECK_PCRE], [
AC_PATH_PROG(pcreconfig,pcre-config)
if test [ -z "$pcreconfig" ]
then
	AC_MSG_ERROR([pcre-config executable not found. Make sure pcre-config is in your path])
else
	AC_MSG_CHECKING([pcre])
	ac_pcre_libs=`${pcreconfig} --libs`
	if test -z "$ac_pcre_libs"
	then
		AC_MSG_RESULT([no])
		AC_MSG_ERROR([Unable to locate pcre libary])
	fi
 
	LDFLAGS="$LDFLAGS $ac_pcre_libs"
	AC_MSG_RESULT([yes])
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
