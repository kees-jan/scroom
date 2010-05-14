AM_CPPFLAGS = @PACKAGE_CFLAGS@ -I $(top_srcdir)/inc

plugin_LTLIBRARIES = libspcolormap.la
libspcolormap_la_SOURCES = colormapplugin.cc colormapplugin.hh colormapprovider.cc colormapprovider.hh colormaps.cc colormaps.hh main.cc
libspcolormap_la_LIBADD = @PACKAGE_LIBS@  -lboost_thread-mt
