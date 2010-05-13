AM_CPPFLAGS = @PACKAGE_CFLAGS@ -I $(top_srcdir)/inc

lib_LTLIBRARIES = libstiledbitmap.la
libstiledbitmap_la_SOURCES = layer.cc layercoordinator.cc layercoordinator.hh layer.hh tiled-bitmap.cc tiled-bitmap.hh tileinternal.cc tileinternal.hh

libstiledbitmap_la_LIBADD = @PACKAGE_LIBS@ -L../threadpool -lsthreadpool
