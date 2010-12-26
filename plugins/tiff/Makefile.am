AM_CPPFLAGS = @TIFFPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

plugin_LTLIBRARIES = libsptiff.la
libsptiff_la_SOURCES = layeroperations.cc layeroperations.hh main.cc tiff.cc tiff.hh tiffpresentation.cc tiffpresentation.hh
libsptiff_la_LIBADD = @TIFFPLUGIN_LIBS@ -L$(top_builddir)/libs/tiled-bitmap -lstiledbitmap -ltiff @BOOSTTHREADLIB@
