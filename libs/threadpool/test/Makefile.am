AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/../src

if BOOST_UNITTESTS

check_PROGRAMS = threadpool-tests

threadpool_tests_SOURCES = \
	main.cc \
	helpers.cc helpers.hh \
	semaphore-tests.cc \
	threadpool-tests.cc \
	threadpool-queue-tests.cc \
	threadpool-queueimpl-tests.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

threadpool_tests_LDADD = @BOOSTTHREADLIB@ ../src/libsthreadpool.la

TESTS = threadpool-tests

endif BOOST_UNITTESTS