## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = scroom

scroom_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.cc callbacks.h \
	view.cc view.hh \
	pluginmanager.hh pluginmanager.cc

scroom_CXXFLAGS = -I ../../../inc

scroom_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)
scroom_LDFLAGS = $(shell pkg-config --libs gthread-2.0)