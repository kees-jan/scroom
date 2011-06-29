AM_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = libstiledbitmap.la
libstiledbitmap_la_SOURCES = \
	layer.cc layer.hh \
	layercoordinator.cc layercoordinator.hh \
	tiled-bitmap.cc tiled-bitmap.hh \
	tiledbitmapviewdata.cc tiledbitmapviewdata.hh \
	tileinternal.cc tileinternal.hh \
	tileviewstate.cc tileviewstate.hh \
	local.hh

libstiledbitmap_la_LIBADD = \
	@TILEDBITMAP_LIBS@ \
	../../threadpool/src/libsthreadpool.la \
	../../memory-manager/libsmemman.la \
	@BOOSTTHREADLIB@

# 	../../util/src/libsutil.la \
#
