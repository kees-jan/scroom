AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/../src

if BOOST_UNITTESTS

check_PROGRAMS = preconditions threadpool-tests

preconditions_SOURCES = \
	main.cc \
	helpers.cc helpers.hh \
	semaphore-tests.cc \
	boost-smart-pointers.cc

threadpool_tests_SOURCES = \
	main.cc \
	helpers.cc helpers.hh \
	function-additor-tests.cc \
	threadpool-tests.cc \
	threadpool-destruction-tests.cc \
	threadpool-queue-tests.cc \
	threadpool-queueimpl-tests.cc

preconditions_LDADD = @BOOSTTHREADLIB@ \
	../src/libsthreadpool.la \
	../../boost-test-helper/src/libsboosttesthelper.la

threadpool_tests_LDADD = @BOOSTTHREADLIB@ \
	../src/libsthreadpool.la \
	../../boost-test-helper/src/libsboosttesthelper.la

TESTS = preconditions threadpool-tests

endif BOOST_UNITTESTS