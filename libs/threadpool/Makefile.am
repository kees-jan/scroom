AM_CPPFLAGS = @PACKAGE_CFLAGS@ -I $(top_srcdir)/inc

lib_LTLIBRARIES = libsthreadpool.la
libsthreadpool_la_SOURCES = sequentially.cc sequentially.hh threadpoolimpl.cc threadpoolimpl.hh threadpool.cc
libsthreadpool_la_LIBADD = @PACKAGE_LIBS@ @LIBS@
