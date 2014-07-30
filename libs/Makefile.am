AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

########################################################################
### libs/boost-test-helper
########################################################################

if BOOST_UNITTESTS

boost_test_helper_noinst_libs = \
	boost-test-helper/src/libsboosttesthelper.la
boost_test_helper_src_libsboosttesthelper_la_SOURCES = \
	boost-test-helper/src/unittest-main.cc
boost_test_helper_src_libsboosttesthelper_la_LIBADD = @BOOSTTESTLIB@

boost_test_helper_check_programs = \
	boost-test-helper/test/tests

boost_test_helper_test_tests_SOURCES = \
	boost-test-helper/test/testlib-tests.cc \
	boost-test-helper/test/main.cc

boost_test_helper_test_tests_LDADD = \
	boost-test-helper/src/libsboosttesthelper.la

boost_test_helper_tests = \
	boost-test-helper/test/tests

endif BOOST_UNITTESTS

########################################################################
### libs/compat-tests
########################################################################

if BOOST_UNITTESTS
if !NEW_BOOST_FUTURES
compat_tests_check_programs = \
	compat-tests/boost-future-test/futures-test

compat_tests_boost_future_test_futures_test_SOURCES = \
	compat-tests/boost-future-test/boost-futures-test.cpp \
	compat-tests/boost-future-test/main.cc

compat_tests_boost_future_test_futures_test_CPPFLAGS = @BOOST_FUTURES_CPPFLAGS@

compat_tests_boost_future_test_futures_test_LDADD = @BOOSTTHREADLIB@ \
	boost-test-helper/src/libsboosttesthelper.la

compat_tests_tests = \
	compat-tests/boost-future-test/futures-test

endif !NEW_BOOST_FUTURES
endif BOOST_UNITTESTS

compat_tests_extra_dist = \
	compat-tests/boost-future-test/README.txt \
	compat-tests/README.txt

########################################################################
### libs/sanity-tests
########################################################################

if BOOST_UNITTESTS

sanity_tests_check_programs = \
	sanity-tests/sanity-test

sanity_tests_sanity_test_SOURCES = \
	sanity-tests/main.cc \
	sanity-tests/basic-assumptions.cc \
	sanity-tests/boost-smart-pointers.cc \
	sanity-tests/boost-bind.cc

sanity_tests_sanity_test_LDADD = @BOOSTTHREADLIB@ \
	boost-test-helper/src/libsboosttesthelper.la

sanity_tests_tests = \
	sanity-tests/sanity-test

endif BOOST_UNITTESTS

sanity_tests_extra_dist = \
	sanity-tests/README.txt

########################################################################
### libs/util
########################################################################

util_src_libsutil_la_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

util_libs = \
	util/src/libsutil.la
util_src_libsutil_la_SOURCES = \
	util/src/counter.cc \
	util/src/gtk-helpers.cc \
	util/src/assertions.cc \
	util/src/progressinterfacehelpers.cc

util_src_libsutil_la_LIBADD = @UTILS_LIBS@ @BOOSTTHREADLIB@

if BOOST_UNITTESTS

util_test_utils_tests_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

util_check_programs = \
	util/test/utils-tests
util_test_utils_tests_SOURCES = \
	util/test/main.cc \
	util/test/observable-tests.cc \
	util/test/counter-tests.cc \
	util/test/bookkeeping-tests.cc \
	util/test/gtkhelper-tests.cc \
	util/test/progressinterfacemultiplexer-tests.cc \
	util/test/progressinterfacebroadcaster-tests.cc \
	util/test/progressstateinterfacestub.hh \
	util/test/progressstateinterfacestub.cc \
	util/test/progressinterfaceconversion-tests.cc

util_test_utils_tests_LDADD = @BOOSTTHREADLIB@ @UTILS_LIBS@  \
	util/src/libsutil.la \
	boost-test-helper/src/libsboosttesthelper.la

util_tests = \
	util/test/utils-tests

endif BOOST_UNITTESTS


########################################################################
### libs/threadpool
########################################################################

threadpool_libs = \
	threadpool/src/libsthreadpool.la
threadpool_src_libsthreadpool_la_SOURCES = \
	threadpool/src/function-additor.cc \
	threadpool/src/threadpoolimpl.cc \
	threadpool/src/queue.cc \
	threadpool/src/queue.hh \
	threadpool/src/async-deleter.cc
threadpool_src_libsthreadpool_la_LIBADD = @BOOSTTHREADLIB@

if BOOST_UNITTESTS

threadpool_check_programs = \
	threadpool/test/threadpool-tests

threadpool_test_threadpool_tests_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ \
	-I $(srcdir)/threadpool/src

threadpool_test_threadpool_tests_SOURCES = \
	threadpool/test/main.cc \
	threadpool/test/helpers.cc \
	threadpool/test/helpers.hh \
	threadpool/test/semaphore-tests.cc \
	threadpool/test/function-additor-tests.cc \
	threadpool/test/threadpool-tests.cc \
	threadpool/test/threadpool-destruction-tests.cc \
	threadpool/test/threadpool-queue-tests.cc \
	threadpool/test/threadpool-queueimpl-tests.cc

threadpool_test_threadpool_tests_LDADD = @BOOSTTHREADLIB@ \
	threadpool/src/libsthreadpool.la \
	boost-test-helper/src/libsboosttesthelper.la

threadpool_tests = \
	threadpool/test/threadpool-tests

endif BOOST_UNITTESTS

########################################################################
### libs/memory-manager
########################################################################

memory_manager_libs = \
	memory-manager/src/libsmemman.la
memory_manager_src_libsmemman_la_SOURCES = \
	memory-manager/src/filebackedmemory.cc \
	memory-manager/src/memorymanager.cc \
	memory-manager/src/swapbasedblockallocator.cc \
	memory-manager/src/blob-compression.hh \
	memory-manager/src/blob-compression.cc \
	memory-manager/src/memoryblobs.cc

memory_manager_src_libsmemman_la_LIBADD = \
	util/src/libsutil.la \
	threadpool/src/libsthreadpool.la \
	@BOOSTTHREADLIB@ \
	@ZLIB@

if BOOST_UNITTESTS

memory_manager_check_programs = \
	memory-manager/test/memory-manager-tests

memory_manager_test_memory_manager_tests_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ \
	-I $(srcdir)/memory-manager/src

memory_manager_test_memory_manager_tests_SOURCES = \
	memory-manager/test/main.cc \
	memory-manager/test/swap-block-allocator-tests.cc \
	memory-manager/test/pageprovider-tests.cc \
	memory-manager/test/compression-tests.cc \
	memory-manager/test/blob-tests.cc

memory_manager_test_memory_manager_tests_LDADD = \
	util/src/libsutil.la \
	memory-manager/src/libsmemman.la \
	boost-test-helper/src/libsboosttesthelper.la \
	@BOOSTTHREADLIB@

memory_manager_tests = \
	memory-manager/test/memory-manager-tests

endif BOOST_UNITTESTS

########################################################################
### libs/scroom
########################################################################

scroom_src_libsscroom_la_CPPFLAGS = @SCROOMLIB_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

scroomlib_libs = \
	scroom/src/libsscroom.la

scroom_src_libsscroom_la_SOURCES = \
	scroom/src/bitmap-helpers.cc

scroom_src_libsscroom_la_LIBADD = \
	@SCROOMLIB_LIBS@ \
	@BOOSTTHREADLIB@

scroomlib_check_programs =
scroomlib_tests = 

########################################################################
### libs/tiled-bitmap
########################################################################

tiled_bitmap_src_libstiledbitmap_la_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

tiled_bitmap_libs = \
	tiled-bitmap/src/libstiledbitmap.la

tiled_bitmap_src_libstiledbitmap_la_SOURCES = \
	tiled-bitmap/src/layer.cc \
	tiled-bitmap/src/layer.hh \
	tiled-bitmap/src/layercoordinator.cc \
	tiled-bitmap/src/layercoordinator.hh \
	tiled-bitmap/src/layeroperations.cc \
	tiled-bitmap/src/tiled-bitmap.cc \
	tiled-bitmap/src/tiled-bitmap.hh \
	tiled-bitmap/src/tiledbitmapviewdata.cc \
	tiled-bitmap/src/tiledbitmapviewdata.hh \
	tiled-bitmap/src/tileinternalobserverinterfaces.cc \
	tiled-bitmap/src/tileinternalobserverinterfaces.hh \
	tiled-bitmap/src/tileinternal.cc \
	tiled-bitmap/src/tileinternal.hh \
	tiled-bitmap/src/tileviewstate.cc \
	tiled-bitmap/src/tileviewstate.hh \
	tiled-bitmap/src/local.hh

tiled_bitmap_src_libstiledbitmap_la_LIBADD = \
	@TILEDBITMAP_LIBS@ \
	util/src/libsutil.la \
	threadpool/src/libsthreadpool.la \
	memory-manager/src/libsmemman.la \
	scroom/src/libsscroom.la \
	@BOOSTTHREADLIB@

if BOOST_UNITTESTS

tiled_bitmap_tests = \
	tiled-bitmap/test/tiled-bitmap-tests

tiled_bitmap_test_tiled_bitmap_tests_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

tiled_bitmap_test_tiled_bitmap_tests_SOURCES = \
	tiled-bitmap/test/main.cc \
	tiled-bitmap/test/tiledbitmap-tests.cc

tiled_bitmap_test_tiled_bitmap_tests_LDADD = \
	@BOOSTTHREADLIB@ \
	tiled-bitmap/src/libstiledbitmap.la \
	util/src/libsutil.la \
	boost-test-helper/src/libsboosttesthelper.la

endif BOOST_UNITTESTS

tiled_bitmap_tools_measure_framerate_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

tiled_bitmap_check_programs = \
	$(tiled_bitmap_tests) \
	tiled-bitmap/tools/measure-framerate \
	tiled-bitmap/tools/measure-load-performance

tiled_bitmap_tools_measure_framerate_SOURCES = \
	tiled-bitmap/tools/measure-framerate.cc \
	tiled-bitmap/tools/test-helpers.cc \
	tiled-bitmap/tools/test-helpers.hh \
	tiled-bitmap/tools/measure-framerate-callbacks.hh \
	tiled-bitmap/tools/measure-framerate-callbacks.cc \
	tiled-bitmap/tools/measure-framerate-tests.hh \
	tiled-bitmap/tools/measure-framerate-tests.cc \
	tiled-bitmap/tools/measure-framerate-stubs.hh \
	tiled-bitmap/tools/measure-framerate-stubs.cc

tiled_bitmap_tools_measure_framerate_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ \
	tiled-bitmap/src/libstiledbitmap.la \
	util/src/libsutil.la

tiled_bitmap_tools_measure_load_performance_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

tiled_bitmap_tools_measure_load_performance_SOURCES = \
	tiled-bitmap/tools/measure-load-performance.cc \
	tiled-bitmap/tools/test-helpers.cc \
	tiled-bitmap/tools/test-helpers.hh \
	tiled-bitmap/tools/measure-load-performance-tests.cc \
	tiled-bitmap/tools/measure-load-performance-tests.hh \
	tiled-bitmap/tools/measure-framerate-callbacks.hh \
	tiled-bitmap/tools/measure-framerate-callbacks.cc \
	tiled-bitmap/tools/measure-framerate-stubs.hh \
	tiled-bitmap/tools/measure-framerate-stubs.cc

tiled_bitmap_tools_measure_load_performance_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ \
	threadpool/src/libsthreadpool.la \
	tiled-bitmap/src/libstiledbitmap.la \
	util/src/libsutil.la

########################################################################
### Wrapping up
########################################################################

lib_LTLIBRARIES = $(boost_test_helper_libs) $(compat_tests_libs) $(sanity_tests_libs) $(util_libs) $(threadpool_libs) $(memory_manager_libs) $(scroomlib_libs) $(tiled_bitmap_libs)
check_PROGRAMS = $(boost_test_helper_check_programs) $(compat_tests_check_programs) $(sanity_tests_check_programs) $(util_check_programs) $(threadpool_check_programs) $(memory_manager_check_programs) $(scroomlib_check_programs) $(tiled_bitmap_check_programs)
TESTS = $(boost_test_helper_tests) $(compat_tests_tests) $(sanity_tests_tests) $(util_tests) $(threadpool_tests) $(memory_manager_tests) $(scroomlib_tests) $(tiled_bitmap_tests)
EXTRA_DIST = $(compat_tests_extra_dist) $(sanity_tests_extra_dist)
noinst_LTLIBRARIES = $(boost_test_helper_noinst_libs)