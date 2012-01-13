AM_CPPFLAGS = -I $(top_srcdir)/inc

if BOOST_UNITTESTS

check_PROGRAMS = tests

tests_SOURCES = \
	testlib-tests.cc \
	main.cc

tests_LDADD = ../src/libsboosttesthelper.la

TESTS = tests

endif BOOST_UNITTESTS
