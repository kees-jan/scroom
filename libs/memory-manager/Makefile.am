AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = src/libsmemman.la
src_libsmemman_la_SOURCES = \
	src/filebackedmemory.cc \
	src/memorymanager.cc \
	src/swapbasedblockallocator.cc \
	src/blob-compression.hh \
	src/blob-compression.cc \
	src/memoryblobs.cc

src_libsmemman_la_LIBADD = ../util/src/libsutil.la ../threadpool/src/libsthreadpool.la @BOOSTTHREADLIB@ @ZLIB@

if BOOST_UNITTESTS

check_PROGRAMS = test/memory-manager-tests

test_memory_manager_tests_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/src

test_memory_manager_tests_SOURCES = \
	test/main.cc \
	test/swap-block-allocator-tests.cc \
	test/pageprovider-tests.cc \
	test/compression-tests.cc \
	test/blob-tests.cc

test_memory_manager_tests_LDADD = \
	../util/src/libsutil.la \
	src/libsmemman.la \
	../boost-test-helper/src/libsboosttesthelper.la \
	@BOOSTTHREADLIB@

TESTS = test/memory-manager-tests

endif BOOST_UNITTESTS
