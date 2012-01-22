AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = libsmemman.la
libsmemman_la_SOURCES = \
	filebackedmemory.cc \
	memorymanager.cc \
	swapbasedblockallocator.cc

libsmemman_la_LIBADD = ../../threadpool/src/libsthreadpool.la @BOOSTTHREADLIB@ @ZLIB@
