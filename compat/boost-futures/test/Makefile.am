## Process this file with automake to produce Makefile.in

check_PROGRAMS = futures-test

futures_test_SOURCES = \
	boost-futures-test.cpp \
	main.cc

futures_test_CPPFLAGS = -I $(top_srcdir)/compat/boost-futures/inc

futures_test_LDADD = -lboost_unit_test_framework

TESTS = futures-test