AM_CPPFLAGS = -I $(top_srcdir)/inc

if BOOST_UNITTESTS

noinst_LTLIBRARIES = libsboosttesthelper.la
libsboosttesthelper_la_SOURCES = \
	unittest-main.cc
libsboosttesthelper_la_LIBADD = @BOOSTTESTLIB@

endif BOOST_UNITTESTS
