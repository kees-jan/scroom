AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@ -I $(srcdir)/../src

if BOOST_UNITTESTS

check_PROGRAMS = preconditions threadpool-tests

preconditions_SOURCES = \
	main.cc \
	function-additor.cc function-additor.hh \
	helpers.cc helpers.hh \
	semaphore-tests.cc \
	function-additor-tests.cc

threadpool_tests_SOURCES = \
	main.cc \
	function-additor.cc function-additor.hh \
	helpers.cc helpers.hh \
	threadpool-tests.cc \
	threadpool-destruction-tests.cc \
	threadpool-queue-tests.cc \
	threadpool-queueimpl-tests.cc

# async-deleter-tests.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

preconditions_LDADD = @BOOSTTHREADLIB@ ../src/libsthreadpool.la
threadpool_tests_LDADD = @BOOSTTHREADLIB@ ../src/libsthreadpool.la

TESTS = preconditions threadpool-tests

endif BOOST_UNITTESTS