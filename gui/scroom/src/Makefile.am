## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = scroom

scroom_SOURCES = \
	main.cc \
	callbacks.cc callbacks.hh \
	view.cc view.hh \
	pluginmanager.hh pluginmanager.cc \
	loader.cc loader.hh

scroom_CXXFLAGS = -I ../../../inc

scroom_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)
