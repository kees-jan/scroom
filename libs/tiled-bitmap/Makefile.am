AM_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc

lib_LTLIBRARIES = libstiledbitmap.la
libstiledbitmap_la_SOURCES = layer.cc layercoordinator.cc layercoordinator.hh layer.hh tiled-bitmap.cc tiled-bitmap.hh tileinternal.cc tileinternal.hh

libstiledbitmap_la_LIBADD = @TILEDBITMAP_LIBS@ -L../threadpool -lsthreadpool
