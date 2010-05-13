AM_CPPFLAGS = @PACKAGE_CFLAGS@ -I $(top_srcdir)/inc

pkglib_LTLIBRARIES = libsptiff.la
libsptiff_la_SOURCES = layeroperations.cc layeroperations.hh main.cc tiff.cc tiff.hh tiffpresentation.cc tiffpresentation.hh
libsptiff_la_LIBADD = @PACKAGE_LIBS@ -L$(top_builddir)/libs/tiled-bitmap -lstiledbitmap
