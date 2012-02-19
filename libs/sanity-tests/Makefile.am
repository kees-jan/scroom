## Process this file with automake to produce Makefile.in

if BOOST_UNITTESTS

check_PROGRAMS = sanity-test

sanity_test_SOURCES = \
	main.cc \
	basic-assumptions.cc \
	boost-smart-pointers.cc

sanity_test_LDADD = @BOOSTTHREADLIB@ \
	../boost-test-helper/src/libsboosttesthelper.la

TESTS = sanity-test

endif BOOST_UNITTESTS

EXTRA_DIST = \
	README.txt

