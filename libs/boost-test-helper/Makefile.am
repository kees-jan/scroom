AM_CPPFLAGS = -I $(top_srcdir)/inc

if BOOST_UNITTESTS

noinst_LTLIBRARIES = src/libsboosttesthelper.la
src_libsboosttesthelper_la_SOURCES = \
	src/unittest-main.cc
src_libsboosttesthelper_la_LIBADD = @BOOSTTESTLIB@

check_PROGRAMS = test/tests

test_tests_SOURCES = \
	test/testlib-tests.cc \
	test/main.cc

test_tests_LDADD = src/libsboosttesthelper.la

TESTS = test/tests

endif BOOST_UNITTESTS
