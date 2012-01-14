## Process this file with automake to produce Makefile.in

if BOOST_UNITTESTS
if !NEW_BOOST_FUTURES
check_PROGRAMS = futures-test

futures_test_SOURCES = \
	boost-futures-test.cpp \
	main.cc

futures_test_CPPFLAGS = @BOOST_FUTURES_CPPFLAGS@

futures_test_LDADD = @BOOSTTHREADLIB@ \
	../../boost-test-helper/src/libsboosttesthelper.la

TESTS = futures-test

endif !NEW_BOOST_FUTURES
endif BOOST_UNITTESTS