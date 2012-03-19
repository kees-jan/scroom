AM_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

lib_LTLIBRARIES = libstiledbitmap.la
libstiledbitmap_la_SOURCES = \
	layer.cc layer.hh \
	layercoordinator.cc layercoordinator.hh \
	layeroperations.cc \
	tiled-bitmap.cc tiled-bitmap.hh \
	tiledbitmapviewdata.cc tiledbitmapviewdata.hh \
	tileinternalobserverinterfaces.cc tileinternalobserverinterfaces.hh \
	tileinternal.cc tileinternal.hh \
	tileviewstate.cc tileviewstate.hh \
	local.hh

libstiledbitmap_la_LIBADD = \
	@TILEDBITMAP_LIBS@ \
	../../util/src/libsutil.la \
	../../threadpool/src/libsthreadpool.la \
	../../memory-manager/src/libsmemman.la \
	@BOOSTTHREADLIB@
