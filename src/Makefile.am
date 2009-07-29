AUTOMAKE_OPTIONS = gnu

bin_PROGRAMS = spmfilter
spmfilter_SOURCES = main.c
spmfilter_LDADD = libspmfilter.la

AM_LDFLAGS = -export-dynamic

pkglib_LTLIBRARIES = libspmfilter.la libsmtpd.la libpipe.la
pkginclude_HEADERS = spmfilter.h
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libspmfilter.pc

COMMON = spmutils.c smtp.c
libspmfilter_la_SOURCES = $(COMMON)
libspmfilter_la_LDFLAGS = $(AM_LDFLAGS)
libspmfilter_la_LIBADD = @LTLIBOBJS@

SMTPD = smtpd.c smtpd.h
libsmtpd_la_SOURCES = $(SMTPD)
libsmtpd_la_LDFLAGS = $(AM_LDFLAGS)
libsmtpd_la_LIBADD = @LTLIBOBJS@

PIPE = pipe.c
libpipe_la_SOURCES = $(PIPE)
libpipe_la_LDFLAGS = $(AM_LDFLAGS)
libpipe_la_LIBADD = @LTLIBOBJS@

