## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DPLUGIN_DIR=\""$(plugindir)"\" \
	-DTOP_SRCDIR=\""$(abs_top_srcdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GUI_CFLAGS@ \
	@BOOST_FUTURES_CPPFLAGS@

bin_PROGRAMS = scroom

scroom_SOURCES = \
	main.cc \
	callbacks.cc callbacks.hh \
	view.cc view.hh \
	pluginmanager.hh pluginmanager.cc \
	loader.cc loader.hh \
	sidebarmanager.cc sidebarmanager.hh \
	progressbarmanager.cc progressbarmanager.hh \
	workinterface.hh

scroom_CPPFLAGS = -I $(top_srcdir)/inc $(AM_CPPFLAGS)

scroom_LDADD = @GUI_LIBS@ @BOOSTTHREADLIB@ $(INTLLIBS)
