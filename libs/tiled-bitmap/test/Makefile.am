AM_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

if BOOST_UNITTESTS

EXTRA_PROGRAMS = tiled-bitmap-tests

TBT = tiled-bitmap-tests

tiled_bitmap_tests_SOURCES = \
	main.cc \
	tiledbitmap-tests.cc

tiled_bitmap_tests_LDADD = \
	@BOOSTTHREADLIB@ \
	../src/libstiledbitmap.la \
	../../util/src/libsutil.la

TESTS = tiled-bitmap-tests

endif BOOST_UNITTESTS

check_PROGRAMS = $(TBT)
