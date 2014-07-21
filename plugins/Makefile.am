AM_CPPFLAGS = -I $(top_srcdir)/inc @BOOST_FUTURES_CPPFLAGS@

########################################################################
### plugins/example
########################################################################

example_plugin_libs = \
	example/libspexample.la

example_libspexample_la_CPPFLAGS = @EXAMPLEPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

example_libspexample_la_SOURCES = \
	example/example.cc \
	example/example.hh \
	example/examplepresentation.cc \
	example/examplepresentation.hh \
	example/main.cc

example_libspexample_la_LIBADD = -lm @EXAMPLEPLUGIN_LIBS@ @BOOSTTHREADLIB@

########################################################################
### plugins/tiff
########################################################################

tiff_plugin_libs = \
	tiff/src/libsptiff.la

tiff_src_libsptiff_la_CPPFLAGS = @TIFFPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

tiff_src_libsptiff_la_SOURCES = \
	tiff/src/main.cc \
	tiff/src/tiff.cc \
	tiff/src/tiff.hh \
	tiff/src/tiffpresentation.cc \
	tiff/src/tiffpresentation.hh

tiff_src_libsptiff_la_LIBADD = @TIFFPLUGIN_LIBS@ \
	../libs/tiled-bitmap/src/libstiledbitmap.la -ltiff @BOOSTTHREADLIB@


########################################################################
### plugins/colormap
########################################################################

colormap_plugin_libs = \
	colormap/libspcolormap.la

colormap_libspcolormap_la_CPPFLAGS = @COLORMAPPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

colormap_libspcolormap_la_SOURCES = \
	colormap/colormapplugin.cc \
	colormap/colormapplugin.hh \
	colormap/colormapprovider.cc \
	colormap/colormapprovider.hh \
	colormap/colormaps.cc \
	colormap/colormaps.hh \
	colormap/main.cc

colormap_libspcolormap_la_LIBADD = @COLORMAPPLUGIN_LIBS@ @BOOSTTHREADLIB@

########################################################################
### Wrapping up
########################################################################

plugin_LTLIBRARIES = $(example_plugin_libs) $(tiff_plugin_libs) $(colormap_plugin_libs)