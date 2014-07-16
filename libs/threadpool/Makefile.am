AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = src/libsthreadpool.la
src_libsthreadpool_la_SOURCES = \
	src/function-additor.cc \
	src/threadpoolimpl.cc \
	src/queue.cc queue.hh \
	src/async-deleter.cc
src_libsthreadpool_la_LIBADD = @BOOSTTHREADLIB@


if BOOST_UNITTESTS

check_PROGRAMS = test/threadpool-tests

test_threadpool_tests_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/src

test_threadpool_tests_SOURCES = \
	test/main.cc \
	test/helpers.cc helpers.hh \
	test/semaphore-tests.cc \
	test/function-additor-tests.cc \
	test/threadpool-tests.cc \
	test/threadpool-destruction-tests.cc \
	test/threadpool-queue-tests.cc \
	test/threadpool-queueimpl-tests.cc

test_threadpool_tests_LDADD = @BOOSTTHREADLIB@ \
	src/libsthreadpool.la \
	../boost-test-helper/src/libsboosttesthelper.la

TESTS = test/threadpool-tests

endif BOOST_UNITTESTS