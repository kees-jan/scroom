AM_CPPFLAGS = @PACKAGE_CFLAGS@ -I $(top_srcdir)/inc

plugin_LTLIBRARIES = libsptiff.la
libsptiff_la_SOURCES = layeroperations.cc layeroperations.hh main.cc tiff.cc tiff.hh tiffpresentation.cc tiffpresentation.hh
libsptiff_la_LIBADD = @PACKAGE_LIBS@ -lboost_thread-mt -L$(top_builddir)/libs/tiled-bitmap -lstiledbitmap -ltiff
