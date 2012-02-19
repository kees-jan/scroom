AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/../src

if BOOST_UNITTESTS

check_PROGRAMS = threadpool-tests

threadpool_tests_SOURCES = \
	main.cc \
	helpers.cc helpers.hh \
	semaphore-tests.cc \
	function-additor-tests.cc \
	threadpool-tests.cc \
	threadpool-destruction-tests.cc \
	threadpool-queue-tests.cc \
	threadpool-queueimpl-tests.cc

threadpool_tests_LDADD = @BOOSTTHREADLIB@ \
	../src/libsthreadpool.la \
	../../boost-test-helper/src/libsboosttesthelper.la

TESTS = threadpool-tests

endif BOOST_UNITTESTS