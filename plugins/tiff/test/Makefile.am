AM_CPPFLAGS = @TIFFPLUGIN_CFLAGS@ -I $(top_srcdir)/inc -I $(top_srcdir)/plugins/tiff/src

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

check_PROGRAMS = $(TBT) measure-framerate

measure_framerate_SOURCES = \
	measure-framerate.cc \
	measure-framerate-callbacks.hh measure-framerate-callbacks.cc \
	measure-framerate-tests.hh measure-framerate-tests.cc \
	measure-framerate-stubs.hh measure-framerate-stubs.cc

measure_framerate_LDADD = \
	@TIFFPLUGIN_LIBS@ -L$(top_builddir)/libs/tiled-bitmap/src -lstiledbitmap @BOOSTTHREADLIB@ \
	../src/libsptiff.la \
	$(top_builddir)/libs/tiled-bitmap/src/libstiledbitmap.la \
	$(top_builddir)/libs/util/src/libsutil.la

