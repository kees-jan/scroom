AM_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

# if BOOST_UNITTESTS
# 
# EXTRA_PROGRAMS = tiled-bitmap-tests
# 
# TBT = tiled-bitmap-tests
# 
# tiled_bitmap_tests_SOURCES = \
# 	main.cc \
# 	tiledbitmap-tests.cc
# 
# tiled_bitmap_tests_LDADD = \
# 	@BOOSTTHREADLIB@ \
# 	../src/libstiledbitmap.la \
# 	../../util/src/libsutil.la
# 
# TESTS = tiled-bitmap-tests
# 
# endif BOOST_UNITTESTS

check_PROGRAMS = $(TBT) measure-framerate measure-load-performance

measure_framerate_SOURCES = \
	measure-framerate.cc \
	test-helpers.cc test-helpers.hh \
	measure-framerate-callbacks.hh measure-framerate-callbacks.cc \
	measure-framerate-tests.hh measure-framerate-tests.cc \
	measure-framerate-stubs.hh measure-framerate-stubs.cc

measure_framerate_LDADD = \
	@GUI_LIBS@ -L$(top_builddir)/libs/tiled-bitmap/src -lstiledbitmap @BOOSTTHREADLIB@ \
	$(top_builddir)/libs/tiled-bitmap/src/libstiledbitmap.la \
	$(top_builddir)/libs/util/src/libsutil.la

measure_load_performance_SOURCES = \
	measure-load-performance.cc \
	test-helpers.cc test-helpers.hh \
	measure-load-performance-tests.cc measure-load-performance-tests.hh \
	measure-framerate-callbacks.hh measure-framerate-callbacks.cc \
	measure-framerate-stubs.hh measure-framerate-stubs.cc

measure_load_performance_LDADD = \
	@GUI_LIBS@ -L$(top_builddir)/libs/tiled-bitmap/src -lstiledbitmap @BOOSTTHREADLIB@ \
	../../../libs/threadpool/src/libsthreadpool.la \
	$(top_builddir)/libs/tiled-bitmap/src/libstiledbitmap.la \
	$(top_builddir)/libs/util/src/libsutil.la
