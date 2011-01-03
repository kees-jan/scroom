AM_CPPFLAGS = -I $(top_srcdir)/inc # @BOOST_FUTURES_CPPFLAGS@

if BOOST_UNITTESTS

check_PROGRAMS = utils-tests

utils_tests_SOURCES = \
	main.cc \
	observable-tests.cc \
	basic-assumptions.cc

# threadpool_tests_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

utils_tests_LDADD = @BOOSTTHREADLIB@

TESTS = utils-tests

endif BOOST_UNITTESTS