AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/../src

if BOOST_UNITTESTS

check_PROGRAMS = memory-manager-tests

memory_manager_tests_SOURCES = \
	main.cc \
	swap-block-allocator-tests.cc

memory_manager_tests_LDADD = \
	../src/libsmemman.la \
	../../boost-test-helper/src/libsboosttesthelper.la

TESTS = memory-manager-tests

endif BOOST_UNITTESTS