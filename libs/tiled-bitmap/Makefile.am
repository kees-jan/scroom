src_libstiledbitmap_la_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = src/libstiledbitmap.la
src_libstiledbitmap_la_SOURCES = \
	src/bitmap-helpers.cc \
	src/layer.cc \
	src/layer.hh \
	src/layercoordinator.cc \
	src/layercoordinator.hh \
	src/layeroperations.cc \
	src/tiled-bitmap.cc \
	src/tiled-bitmap.hh \
`	src/tiledbitmapviewdata.cc \
	src/tiledbitmapviewdata.hh \
	src/tileinternalobserverinterfaces.cc \
	src/tileinternalobserverinterfaces.hh \
	src/tileinternal.cc \
	src/tileinternal.hh \
	src/tileviewstate.cc \
	src/tileviewstate.hh \
	src/local.hh

src_libstiledbitmap_la_LIBADD = \
	@TILEDBITMAP_LIBS@ \
	../util/src/libsutil.la \
	../threadpool/src/libsthreadpool.la \
	../memory-manager/src/libsmemman.la \
	@BOOSTTHREADLIB@

if BOOST_UNITTESTS

tiled_bitmap_unittests = test/tiled-bitmap-tests

test_tiled_bitmap_tests_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

test_tiled_bitmap_tests_SOURCES = \
	test/main.cc \
	test/tiledbitmap-tests.cc

test_tiled_bitmap_tests_LDADD = \
	@BOOSTTHREADLIB@ \
	src/libstiledbitmap.la \
	../util/src/libsutil.la \
	../boost-test-helper/src/libsboosttesthelper.la

TESTS = test/tiled-bitmap-tests

endif BOOST_UNITTESTS

tools_measure_framerate_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

check_PROGRAMS = $(tiled_bitmap_unittests) tools/measure-framerate tools/measure-load-performance

tools_measure_framerate_SOURCES = \
	tools/measure-framerate.cc \
	tools/test-helpers.cc \
	tools/test-helpers.hh \
	tools/measure-framerate-callbacks.hh \
	tools/measure-framerate-callbacks.cc \
	tools/measure-framerate-tests.hh \
	tools/measure-framerate-tests.cc \
	tools/measure-framerate-stubs.hh \
	tools/measure-framerate-stubs.cc

tools_measure_framerate_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ \
	$(top_builddir)/libs/tiled-bitmap/src/libstiledbitmap.la \
	$(top_builddir)/libs/util/src/libsutil.la

tools_measure_load_performance_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

tools_measure_load_performance_SOURCES = \
	tools/measure-load-performance.cc \
	tools/test-helpers.cc \
	tools/test-helpers.hh \
	tools/measure-load-performance-tests.cc \
	tools/measure-load-performance-tests.hh \
	tools/measure-framerate-callbacks.hh \
	tools/measure-framerate-callbacks.cc \
	tools/measure-framerate-stubs.hh \
	tools/measure-framerate-stubs.cc

tools_measure_load_performance_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ \
	../../libs/threadpool/src/libsthreadpool.la \
	$(top_builddir)/libs/tiled-bitmap/src/libstiledbitmap.la \
	$(top_builddir)/libs/util/src/libsutil.la
