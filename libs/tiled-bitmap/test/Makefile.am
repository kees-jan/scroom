AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

if BOOST_UNITTESTS

check_PROGRAMS = tiled-bitmap-tests

tiled_bitmap_tests_SOURCES = \
	main.cc \
	tiledbitmap-tests.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

tiled_bitmap_tests_LDADD = @BOOSTTHREADLIB@ ../src/libstiledbitmap.la

TESTS = tiled-bitmap-tests

endif BOOST_UNITTESTS