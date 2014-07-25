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
### plugins/transparentoverlay
########################################################################

transparent_overlay_plugin_libs = \
	transparent-overlay/src/libsptransparentoverlay.la

transparent_overlay_src_libsptransparentoverlay_la_CPPFLAGS = @TRANSPARENTOVERLAYPLUGIN_CFLAGS@ \
	-I $(top_srcdir)/inc

transparent_overlay_src_libsptransparentoverlay_la_SOURCES = \
	transparent-overlay/src/transparentoverlay.cc \
	transparent-overlay/src/transparentoverlay.hh \
	transparent-overlay/src/transparentoverlaypresentation.cc \
	transparent-overlay/src/transparentoverlaypresentation.hh \
	transparent-overlay/src/transparentoverlayviewinfo.cc \
	transparent-overlay/src/transparentoverlayviewinfo.hh \
	transparent-overlay/src/main.cc

transparent_overlay_src_libsptransparentoverlay_la_LIBADD = -lm @TRANSPARENTOVERLAYPLUGIN_LIBS@ @BOOSTTHREADLIB@ \
	../libs/util/src/libsutil.la

if BOOST_UNITTESTS

transparent_overlay_test_overlay_tests_CPPFLAGS = @TRANSPARENTOVERLAYPLUGIN_CFLAGS@ \
	-I $(top_srcdir)/inc

transparent_overlay_plugin_check_programs = \
	transparent-overlay/test/overlay-tests
transparent_overlay_test_overlay_tests_SOURCES = \
	transparent-overlay/test/main.cc \
	transparent-overlay/test/plugin-test.cc

transparent_overlay_test_overlay_tests_LDADD = @BOOSTTHREADLIB@ @UTILS_LIBS@  \
	transparent-overlay/src/libsptransparentoverlay.la \
	../libs/boost-test-helper/src/libsboosttesthelper.la

transparent_overlay_plugin_tests = \
	transparent-overlay/test/overlay-tests

endif BOOST_UNITTESTS


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

plugin_LTLIBRARIES = $(example_plugin_libs) $(transparent_overlay_plugin_libs) $(tiff_plugin_libs) $(colormap_plugin_libs)
check_PROGRAMS = $(transparent_overlay_plugin_check_programs)
TESTS = $(transparent_overlay_plugin_tests)