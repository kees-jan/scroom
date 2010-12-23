AM_CPPFLAGS = -I $(top_srcdir)/inc

if BOOST_UNITTESTS

check_PROGRAMS = threadpool-tests

threadpool_tests_SOURCES = \
	main.cc \
	semaphore-tests.cc \
	threadpool-tests.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

threadpool_tests_LDADD = @BOOSTTHREADLIB@ ../src/libsthreadpool.la

TESTS = threadpool-tests

endif BOOST_UNITTESTS