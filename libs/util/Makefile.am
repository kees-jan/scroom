AM_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

lib_LTLIBRARIES = src/libsutil.la
src_libsutil_la_SOURCES = \
	src/counter.cc \
	src/gtk-helpers.cc \
	src/assertions.cc \
	src/progressinterfacehelpers.cc


src_libsutil_la_LIBADD = @UTILS_LIBS@ @BOOSTTHREADLIB@

if BOOST_UNITTESTS

check_PROGRAMS = test/utils-tests

test_utils_tests_SOURCES = \
	test/main.cc \
	test/observable-tests.cc \
	test/counter-tests.cc \
	test/bookkeeping-tests.cc \
	test/gtkhelper-tests.cc \
	test/progressinterfacemultiplexer-tests.cc \
	test/progressinterfacebroadcaster-tests.cc \
	test/progressstateinterfacestub.hh \
`	test/progressstateinterfacestub.cc \
	test/progressinterfaceconversion-tests.cc

test_utils_tests_LDADD = @BOOSTTHREADLIB@ @UTILS_LIBS@  \
	src/libsutil.la \
	../boost-test-helper/src/libsboosttesthelper.la

TESTS = test/utils-tests

endif BOOST_UNITTESTS
