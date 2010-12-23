AM_CPPFLAGS = -I $(top_srcdir)/inc -I $(top_srcdir)/compat/boost-futures/inc

lib_LTLIBRARIES = libsthreadpool.la
libsthreadpool_la_SOURCES = threadpoolimpl.cc threadpool.cc
libsthreadpool_la_LIBADD = @BOOSTTHREADLIB@
