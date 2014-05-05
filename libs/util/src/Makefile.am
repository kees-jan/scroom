AM_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

lib_LTLIBRARIES = libsutil.la
libsutil_la_SOURCES = \
	counter.cc \
	gtk-helpers.cc \
	assertions.cc \
	progressinterfacemultiplexer.cc \
	progressinterfacehelpers.cc


libsutil_la_LIBADD = @UTILS_LIBS@ @BOOSTTHREADLIB@
