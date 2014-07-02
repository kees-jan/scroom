AM_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc # @BOOST_FUTURES_CPPFLAGS@

if BOOST_UNITTESTS

check_PROGRAMS = utils-tests

utils_tests_SOURCES = \
	main.cc \
	observable-tests.cc \
	counter-tests.cc \
	bookkeeping-tests.cc \
	gtkhelper-tests.cc \
	progressinterfacemultiplexer-tests.cc \
	progressinterfacebroadcaster-tests.cc \
	progressstateinterfacestub.hh progressstateinterfacestub.cc \
	progressinterfaceconversion-tests.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

utils_tests_LDADD = @BOOSTTHREADLIB@ @UTILS_LIBS@  \
	../src/libsutil.la \
	../../boost-test-helper/src/libsboosttesthelper.la

TESTS = utils-tests

endif BOOST_UNITTESTS