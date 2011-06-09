AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = libsthreadpool.la
libsthreadpool_la_SOURCES = \
	threadpoolimpl.cc \
	threadpool.cc \
	queue.cc queue.hh \
	async-deleter.cc
libsthreadpool_la_LIBADD = @BOOSTTHREADLIB@
