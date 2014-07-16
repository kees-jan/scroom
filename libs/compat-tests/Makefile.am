if BOOST_UNITTESTS
if !NEW_BOOST_FUTURES
check_PROGRAMS = boost-future-test/futures-test

boost_future_test_futures_test_SOURCES = \
	boost-future-test/boost-futures-test.cpp \
	boost-future-test/main.cc

boost_future_test_futures_test_CPPFLAGS = @BOOST_FUTURES_CPPFLAGS@

boost_future_test_futures_test_LDADD = @BOOSTTHREADLIB@ \
	../boost-test-helper/src/libsboosttesthelper.la

TESTS = boost-future-test/futures-test

endif !NEW_BOOST_FUTURES
endif BOOST_UNITTESTS

EXTRA_DIST = \
	boost-future-test/README.txt \
	README.txt


