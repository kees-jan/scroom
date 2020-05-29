include $(top_srcdir)/aminclude/doxygen.am

AM_CPPFLAGS = -I $(top_srcdir)/inc
ACLOCAL_AMFLAGS = -I m4

########################################################################
### inc
########################################################################

incscroom_HEADERS = \
	inc/scroom/assertions.hh \
	inc/scroom/async-deleter.hh \
	inc/scroom/bitmap-helpers.hh \
	inc/scroom/blockallocator.hh \
	inc/scroom/bookkeeping.hh \
	inc/scroom/cairo-helpers.hh \
	inc/scroom/color.hh \
	inc/scroom/colormappable.hh \
	inc/scroom/compiler.hh \
	inc/scroom/dont-delete.hh \
	inc/scroom/function-additor.hh \
	inc/scroom/global.hh \
	inc/scroom/gtk-helpers.hh \
	inc/scroom/layeroperations.hh \
	inc/scroom/linearsegment.hh \
	inc/scroom/memoryblobs.hh \
	inc/scroom/observable.hh \
	inc/scroom/plugininformationinterface.hh \
	inc/scroom/point.hh \
	inc/scroom/presentationinterface.hh \
	inc/scroom/progressinterface.hh \
	inc/scroom/progressinterfacehelpers.hh \
	inc/scroom/rectangle.hh \
	inc/scroom/resizablepresentationinterface.hh \
	inc/scroom/scroominterface.hh \
	inc/scroom/scroomplugin.hh \
	inc/scroom/semaphore.hh \
	inc/scroom/stuff.hh \
	inc/scroom/threadpool.hh \
	inc/scroom/tile.hh \
	inc/scroom/tiledbitmapinterface.hh \
	inc/scroom/tiledbitmaplayer.hh \
	inc/scroom/timer.hh \
	inc/scroom/transformpresentation.hh \
	inc/scroom/unused.hh \
	inc/scroom/utilities.hh \
	inc/scroom/viewinterface.hh

incscroomimpl_HEADERS = \
	inc/scroom/impl/bookkeepingimpl.hh \
	inc/scroom/impl/threadpoolimpl.hh

incscroomgmock_HEADERS = \
	inc/scroom/gmock/colormappable.hh \
	inc/scroom/gmock/presentationinterface.hh

########################################################################
### libs/boost-test-helper
########################################################################

if BOOST_UNITTESTS

boost_test_helper_noinst_libs = \
	libs/boost-test-helper/src/libsboosttesthelper.la
libs_boost_test_helper_src_libsboosttesthelper_la_SOURCES = \
	libs/boost-test-helper/src/unittest-main.cc
libs_boost_test_helper_src_libsboosttesthelper_la_LIBADD = @BOOSTTESTLIB@

boost_test_helper_check_programs = \
	libs/boost-test-helper/test/tests

libs_boost_test_helper_test_tests_SOURCES = \
	libs/boost-test-helper/test/testlib-tests.cc \
	libs/boost-test-helper/test/main.cc

libs_boost_test_helper_test_tests_LDADD = \
	libs/boost-test-helper/src/libsboosttesthelper.la

boost_test_helper_tests = \
	libs/boost-test-helper/test/tests

endif BOOST_UNITTESTS

if GOOGLE_UNITTESTS

google_dir = .google
gtest_dir = $(google_dir)/gtest
gtest_lib = $(gtest_dir)/libgtest.a
gtest_main_lib = $(gtest_dir)/libgtest_main.a

gmock_dir = $(google_dir)/gmock
gmock_lib = $(gmock_dir)/libgmock.a
gmock_main_lib = $(gmock_dir)/libgmock_main.a

$(gtest_lib) $(gtest_main_lib) :
	$(AM_V_GEN) mkdir -p $(gtest_dir) && \
	cd $(gtest_dir) && \
	CC="$(CC)" CXX="$(CXX)" CPPFLAGS="$(CPPFLAGS)" CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" cmake $(GTEST_LOCATION) && \
	$(MAKE)

$(gmock_lib) $(gmock_main_lib) :
	$(AM_V_GEN) mkdir -p $(gmock_dir) && \
	cd $(gmock_dir) && \
	CC="$(CC)" CXX="$(CXX)" CPPFLAGS="$(CPPFLAGS)" CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" cmake $(GMOCK_LOCATION) && \
	$(MAKE)

clean-local:
	-rm -rf $(google_dir)

endif GOOGLE_UNITTESTS

########################################################################
### libs/sanity-tests
########################################################################

if BOOST_UNITTESTS

sanity_tests_check_programs = \
	libs/sanity-tests/sanity-test

libs_sanity_tests_sanity_test_SOURCES = \
	libs/sanity-tests/main.cc \
	libs/sanity-tests/basic-assumptions.cc \
	libs/sanity-tests/boost-smart-pointers.cc \
	libs/sanity-tests/boost-bind.cc

libs_sanity_tests_sanity_test_LDADD = @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/boost-test-helper/src/libsboosttesthelper.la

sanity_tests_tests = \
	libs/sanity-tests/sanity-test

endif BOOST_UNITTESTS

sanity_tests_extra_dist = \
	libs/sanity-tests/README.txt

########################################################################
### libs/util
########################################################################

libs_util_src_libsutil_la_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

util_libs = \
	libs/util/src/libsutil.la
libs_util_src_libsutil_la_SOURCES = \
	libs/util/src/assertions.cc \
	libs/util/src/counter.cc \
	libs/util/src/gtk-helpers.cc \
	libs/util/src/progressinterfacehelpers.cc

libs_util_src_libsutil_la_LIBADD = @UTILS_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@

libs_util_src_libsutil_la_LDFLAGS = $(loadflag)

if BOOST_UNITTESTS

libs_util_test_utils_tests_CPPFLAGS = @UTILS_CFLAGS@ -I $(top_srcdir)/inc

util_check_programs = \
	libs/util/test/utils-tests
libs_util_test_utils_tests_SOURCES = \
	libs/util/test/bookkeeping-tests.cc \
	libs/util/test/counter-tests.cc \
	libs/util/test/gtkhelper-tests.cc \
	libs/util/test/main.cc \
	libs/util/test/observable-tests.cc \
	libs/util/test/progressinterfacebroadcaster-tests.cc \
	libs/util/test/progressinterfaceconversion-tests.cc \
	libs/util/test/progressinterfacemultiplexer-tests.cc \
	libs/util/test/progressstateinterfacestub.cc \
	libs/util/test/progressstateinterfacestub.hh \
	libs/util/test/rectangletests.cc

libs_util_test_utils_tests_LDADD = @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ @UTILS_LIBS@  \
	libs/util/src/libsutil.la \
	libs/boost-test-helper/src/libsboosttesthelper.la

util_tests = \
	libs/util/test/utils-tests

endif BOOST_UNITTESTS


########################################################################
### libs/threadpool
########################################################################

threadpool_libs = \
	libs/threadpool/src/libsthreadpool.la
libs_threadpool_src_libsthreadpool_la_SOURCES = \
	libs/threadpool/src/function-additor.cc \
	libs/threadpool/src/threadpoolimpl.cc \
	libs/threadpool/src/queue.cc \
	libs/threadpool/src/queue.hh \
	libs/threadpool/src/async-deleter.cc
libs_threadpool_src_libsthreadpool_la_LIBADD = @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@

libs_threadpool_src_libsthreadpool_la_LDFLAGS = $(loadflag)

if BOOST_UNITTESTS

threadpool_check_programs = \
	libs/threadpool/test/threadpool-tests

libs_threadpool_test_threadpool_tests_CPPFLAGS = -I $(top_srcdir)/inc \
	-I $(top_srcdir)/libs/threadpool/src

libs_threadpool_test_threadpool_tests_SOURCES = \
	libs/threadpool/test/main.cc \
	libs/threadpool/test/async-deleter-tests.cc \
	libs/threadpool/test/helpers.cc \
	libs/threadpool/test/helpers.hh \
	libs/threadpool/test/semaphore-tests.cc \
	libs/threadpool/test/function-additor-tests.cc \
	libs/threadpool/test/threadpool-tests.cc \
	libs/threadpool/test/threadpool-destruction-tests.cc \
	libs/threadpool/test/threadpool-queue-tests.cc \
	libs/threadpool/test/threadpool-queueimpl-tests.cc

libs_threadpool_test_threadpool_tests_LDADD = @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/threadpool/src/libsthreadpool.la \
	libs/boost-test-helper/src/libsboosttesthelper.la

threadpool_tests = \
	libs/threadpool/test/threadpool-tests

endif BOOST_UNITTESTS

########################################################################
### libs/memory-manager
########################################################################

memory_manager_libs = \
	libs/memory-manager/src/libsmemman.la
libs_memory_manager_src_libsmemman_la_SOURCES = \
	libs/memory-manager/src/swapbasedblockallocator.cc \
	libs/memory-manager/src/blob-compression.hh \
	libs/memory-manager/src/blob-compression.cc \
	libs/memory-manager/src/memoryblobs.cc

libs_memory_manager_src_libsmemman_la_LIBADD = \
	libs/util/src/libsutil.la \
	libs/threadpool/src/libsthreadpool.la \
	@BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	@ZLIB@
	
libs_memory_manager_src_libsmemman_la_LDFLAGS = $(loadflag)	

if BOOST_UNITTESTS

memory_manager_check_programs = \
	libs/memory-manager/test/memory-manager-tests

libs_memory_manager_test_memory_manager_tests_CPPFLAGS = -I $(top_srcdir)/inc \
	-I $(srcdir)/libs/memory-manager/src

libs_memory_manager_test_memory_manager_tests_SOURCES = \
	libs/memory-manager/test/main.cc \
	libs/memory-manager/test/swap-block-allocator-tests.cc \
	libs/memory-manager/test/pageprovider-tests.cc \
	libs/memory-manager/test/compression-tests.cc \
	libs/memory-manager/test/blob-tests.cc

libs_memory_manager_test_memory_manager_tests_LDADD = \
	libs/util/src/libsutil.la \
	libs/memory-manager/src/libsmemman.la \
	libs/boost-test-helper/src/libsboosttesthelper.la \
	@BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@

memory_manager_tests = \
	libs/memory-manager/test/memory-manager-tests
	

endif BOOST_UNITTESTS

########################################################################
### libs/scroom
########################################################################

libs_scroom_src_libsscroom_la_CPPFLAGS = @SCROOMLIB_CFLAGS@ -I $(top_srcdir)/inc

scroomlib_libs = \
	libs/scroom/src/libsscroom.la

libs_scroom_src_libsscroom_la_SOURCES = \
	libs/scroom/src/bitmap-helpers.cc \
	libs/scroom/src/cairo-helpers.cc \
	libs/scroom/src/colormap-helpers.cc \
	libs/scroom/src/presentationinterface.cc \
	libs/scroom/src/transformpresentation.cc

libs_scroom_src_libsscroom_la_LIBADD = \
	@SCROOMLIB_LIBS@ $(util_libs) \
	@BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@

libs_scroom_src_libsscroom_la_LDFLAGS = $(loadflag)

if BOOST_UNITTESTS

scroomlib_check_programs = \
	libs/scroom/test/libscroom-tests

libs_scroom_test_libscroom_tests_CPPFLAGS = @SCROOMLIB_CFLAGS@ -I $(top_srcdir)/inc

libs_scroom_test_libscroom_tests_SOURCES = \
	libs/scroom/test/main.cc \
	libs/scroom/test/colormaphelpers_test.cc

libs_scroom_test_libscroom_tests_LDADD = @BOOSTSYSTEMLIB@ \
	libs/scroom/src/libsscroom.la \
	libs/boost-test-helper/src/libsboosttesthelper.la

scroomlib_tests = \
	libs/scroom/test/libscroom-tests

endif BOOST_UNITTESTS

if GOOGLE_UNITTESTS

scroomlib_gcheck_programs = \
	libs/scroom/test/libscroom-gtests

libs_scroom_test_libscroom_gtests_CPPFLAGS = @SCROOMLIB_CFLAGS@ -I $(top_srcdir)/inc

libs_scroom_test_libscroom_gtests_SOURCES = \
	libs/scroom/test/transformpresentation_test.cc

libs_scroom_test_libscroom_gtests_LDADD = @BOOSTSYSTEMLIB@ \
	libs/scroom/src/libsscroom.la \
	$(gmock_main_lib) $(gtest_lib) \
	@PTHREADLIB@ @SCROOMLIB_LIBS@

scroomlib_gtests = \
	libs/scroom/test/libscroom-gtests

endif GOOGLE_UNITTESTS

########################################################################
### libs/tiled-bitmap
########################################################################

libs_tiled_bitmap_src_libstiledbitmap_la_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc

tiled_bitmap_libs = \
	libs/tiled-bitmap/src/libstiledbitmap.la

libs_tiled_bitmap_src_libstiledbitmap_la_SOURCES = \
	libs/tiled-bitmap/src/layer.cc \
	libs/tiled-bitmap/src/layercoordinator.cc \
	libs/tiled-bitmap/src/layercoordinator.hh \
	libs/tiled-bitmap/src/layeroperations.cc \
	libs/tiled-bitmap/src/cmyklayeroperations.cc \
	libs/tiled-bitmap/src/tiled-bitmap.cc \
	libs/tiled-bitmap/src/tiled-bitmap.hh \
	libs/tiled-bitmap/src/tiledbitmapviewdata.cc \
	libs/tiled-bitmap/src/tiledbitmapviewdata.hh \
	libs/tiled-bitmap/src/compressedtile.cc \
	libs/tiled-bitmap/src/tileviewstate.cc \
	libs/tiled-bitmap/src/tileviewstate.hh \
	libs/tiled-bitmap/src/local.hh

libs_tiled_bitmap_src_libstiledbitmap_la_LIBADD = \
	@TILEDBITMAP_LIBS@ \
	libs/util/src/libsutil.la \
	libs/threadpool/src/libsthreadpool.la \
	libs/memory-manager/src/libsmemman.la \
	libs/scroom/src/libsscroom.la \
	@BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@
	
libs_tiled_bitmap_src_libstiledbitmap_la_LDFLAGS = $(loadflag)

if BOOST_UNITTESTS

tiled_bitmap_tests = \
	libs/tiled-bitmap/test/tiled-bitmap-tests

libs_tiled_bitmap_test_tiled_bitmap_tests_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc

libs_tiled_bitmap_test_tiled_bitmap_tests_SOURCES = \
	libs/tiled-bitmap/test/main.cc \
	libs/tiled-bitmap/test/tiledbitmap-tests.cc

libs_tiled_bitmap_test_tiled_bitmap_tests_LDADD = \
	@BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/tiled-bitmap/src/libstiledbitmap.la \
	libs/util/src/libsutil.la \
	libs/scroom/src/libsscroom.la \
	libs/boost-test-helper/src/libsboosttesthelper.la

endif BOOST_UNITTESTS

libs_tiled_bitmap_tools_measure_framerate_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc

tiled_bitmap_check_programs = \
	$(tiled_bitmap_tests) \
	libs/tiled-bitmap/tools/measure-framerate \
	libs/tiled-bitmap/tools/measure-load-performance

libs_tiled_bitmap_tools_measure_framerate_SOURCES = \
	libs/tiled-bitmap/tools/measure-framerate.cc \
	libs/tiled-bitmap/tools/test-helpers.cc \
	libs/tiled-bitmap/tools/test-helpers.hh \
	libs/tiled-bitmap/tools/measure-framerate-callbacks.hh \
	libs/tiled-bitmap/tools/measure-framerate-callbacks.cc \
	libs/tiled-bitmap/tools/measure-framerate-tests.hh \
	libs/tiled-bitmap/tools/measure-framerate-tests.cc \
	libs/tiled-bitmap/tools/measure-framerate-stubs.hh \
	libs/tiled-bitmap/tools/measure-framerate-stubs.cc

libs_tiled_bitmap_tools_measure_framerate_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/tiled-bitmap/src/libstiledbitmap.la \
	libs/util/src/libsutil.la \
	libs/scroom/src/libsscroom.la

libs_tiled_bitmap_tools_measure_load_performance_CPPFLAGS = @TILEDBITMAP_CFLAGS@ -I $(top_srcdir)/inc

libs_tiled_bitmap_tools_measure_load_performance_SOURCES = \
	libs/tiled-bitmap/tools/measure-load-performance.cc \
	libs/tiled-bitmap/tools/test-helpers.cc \
	libs/tiled-bitmap/tools/test-helpers.hh \
	libs/tiled-bitmap/tools/measure-load-performance-tests.cc \
	libs/tiled-bitmap/tools/measure-load-performance-tests.hh \
	libs/tiled-bitmap/tools/measure-framerate-callbacks.hh \
	libs/tiled-bitmap/tools/measure-framerate-callbacks.cc \
	libs/tiled-bitmap/tools/measure-framerate-stubs.hh \
	libs/tiled-bitmap/tools/measure-framerate-stubs.cc

libs_tiled_bitmap_tools_measure_load_performance_LDADD = \
	@GUI_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/threadpool/src/libsthreadpool.la \
	libs/tiled-bitmap/src/libstiledbitmap.la \
	libs/util/src/libsutil.la \
	libs/scroom/src/libsscroom.la
	

########################################################################
### plugins/example
########################################################################

example_plugin_libs = \
	plugins/example/libspexample.la

plugins_example_libspexample_la_CPPFLAGS = @EXAMPLEPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

plugins_example_libspexample_la_SOURCES = \
	plugins/example/example.cc \
	plugins/example/example.hh \
	plugins/example/examplepresentation.cc \
	plugins/example/examplepresentation.hh \
	plugins/example/main.cc

plugins_example_libspexample_la_LIBADD = -lm @EXAMPLEPLUGIN_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/scroom/src/libsscroom.la
	
plugins_example_libspexample_la_LDFLAGS = $(loadflag)

########################################################################
### plugins/transparentoverlay
########################################################################

transparent_overlay_plugin_libs = \
	plugins/transparent-overlay/src/libsptransparentoverlay.la

plugins_transparent_overlay_src_libsptransparentoverlay_la_CPPFLAGS = @TRANSPARENTOVERLAYPLUGIN_CFLAGS@ \
	-I $(top_srcdir)/inc

plugins_transparent_overlay_src_libsptransparentoverlay_la_SOURCES = \
	plugins/transparent-overlay/src/transparentoverlay.cc \
	plugins/transparent-overlay/src/transparentoverlay.hh \
	plugins/transparent-overlay/src/transparentoverlaypresentation.cc \
	plugins/transparent-overlay/src/transparentoverlaypresentation.hh \
	plugins/transparent-overlay/src/transparentoverlayviewinfo.cc \
	plugins/transparent-overlay/src/transparentoverlayviewinfo.hh \
	plugins/transparent-overlay/src/sizedeterminer.hh \
	plugins/transparent-overlay/src/sizedeterminer.cc \
	plugins/transparent-overlay/src/main.cc

plugins_transparent_overlay_src_libsptransparentoverlay_la_LIBADD = -lm @TRANSPARENTOVERLAYPLUGIN_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/util/src/libsutil.la \
	libs/scroom/src/libsscroom.la

plugins_transparent_overlay_src_libsptransparentoverlay_la_LDFLAGS = $(loadflag)

if BOOST_UNITTESTS

plugins_transparent_overlay_test_overlay_tests_CPPFLAGS = @TRANSPARENTOVERLAYPLUGIN_CFLAGS@ \
	-I $(top_srcdir)/inc -I $(top_srcdir)/plugins/transparent-overlay/src

transparent_overlay_plugin_check_programs = \
	plugins/transparent-overlay/test/overlay-tests
plugins_transparent_overlay_test_overlay_tests_SOURCES = \
	plugins/transparent-overlay/test/main.cc \
	plugins/transparent-overlay/test/plugin-test.cc \
	plugins/transparent-overlay/test/determine-size-test.cc

plugins_transparent_overlay_test_overlay_tests_LDADD = @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ @UTILS_LIBS@ @TRANSPARENTOVERLAY_BOOSTTEST_WIN_LD_SIZEDETERMINER@ \
	plugins/transparent-overlay/src/libsptransparentoverlay.la \
	libs/util/src/libsutil.la \
	libs/scroom/src/libsscroom.la \
	libs/boost-test-helper/src/libsboosttesthelper.la

transparent_overlay_plugin_tests = \
	plugins/transparent-overlay/test/overlay-tests

endif BOOST_UNITTESTS

########################################################################
### plugins/tiff
########################################################################

tiff_plugin_libs = \
	plugins/tiff/src/libsptiff.la

plugins_tiff_src_libsptiff_la_CPPFLAGS = @TIFFPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

plugins_tiff_src_libsptiff_la_SOURCES = \
	plugins/tiff/src/main.cc \
	plugins/tiff/src/tiff.cc \
	plugins/tiff/src/tiff.hh \
	plugins/tiff/src/tiffpresentation.cc \
	plugins/tiff/src/tiffpresentation.hh

plugins_tiff_src_libsptiff_la_LIBADD = @TIFFPLUGIN_LIBS@ -ltiff @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ \
	libs/tiled-bitmap/src/libstiledbitmap.la libs/scroom/src/libsscroom.la
	
plugins_tiff_src_libsptiff_la_LDFLAGS = $(loadflag)

########################################################################
### plugins/colormap
########################################################################

colormap_plugin_libs = \
	plugins/colormap/libspcolormap.la

plugins_colormap_libspcolormap_la_CPPFLAGS = @COLORMAPPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

plugins_colormap_libspcolormap_la_SOURCES = \
	plugins/colormap/colormapplugin.cc \
	plugins/colormap/colormapplugin.hh \
	plugins/colormap/colormapprovider.cc \
	plugins/colormap/colormapprovider.hh \
	plugins/colormap/colormaps.cc \
	plugins/colormap/colormaps.hh \
	plugins/colormap/main.cc

plugins_colormap_libspcolormap_la_LIBADD = @COLORMAPPLUGIN_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@

plugins_colormap_libspcolormap_la_LDFLAGS = $(loadflag)

########################################################################
### plugins/measure
########################################################################

measure_plugin_libs = \
	plugins/measure/src/libspmeasure.la

plugins_measure_src_libspmeasure_la_CPPFLAGS = @MEASUREPLUGIN_CFLAGS@ -I $(top_srcdir)/inc

plugins_measure_src_libspmeasure_la_SOURCES = \
	plugins/measure/src/main.cc \
	plugins/measure/src/measure.cc \
	plugins/measure/src/measure.hh

plugins_measure_src_libspmeasure_la_LIBADD = @MEASUREPLUGIN_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@
	
plugins_measure_src_libspmeasure_la_LDFLAGS = $(loadflag)

########################################################################
### gui
########################################################################

gui_bin_programs = gui/src/scroom

gui_src_scroom_SOURCES = \
	gui/src/main.cc \
	gui/src/callbacks.cc \
	gui/src/callbacks.hh \
	gui/src/view.cc \
	gui/src/view.hh \
	gui/src/pluginmanager.hh \
	gui/src/pluginmanager.cc \
	gui/src/loader.cc \
	gui/src/loader.hh \
	gui/src/sidebarmanager.cc \
	gui/src/sidebarmanager.hh \
	gui/src/progressbarmanager.cc \
	gui/src/progressbarmanager.hh \
	gui/src/workinterface.hh

gui_src_scroom_CPPFLAGS = -I $(top_srcdir)/inc \
	-DPACKAGE_DATA_DIR=\""$(pkgdatadir)"\" \
	-DPLUGIN_DIR=\""$(plugindir)"\" \
	-DTOP_SRCDIR=\""$(abs_top_srcdir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@GUI_CFLAGS@

gui_src_scroom_LDADD = @GUI_LIBS@ @BOOSTTHREADLIB@ @BOOSTSYSTEMLIB@ @BOOSTPROGRAMOPTIONSLIB@ @BOOSTFILESYSTEMLIB@ $(INTLLIBS) \
	libs/util/src/libsutil.la

gui_extra_dist = \
	gui/scroom.glade \
	gui/ENVIRONMENT

gui_pkgdata_data = \
	gui/scroom.glade

########################################################################
### Root/
########################################################################

root_extra_dist = \
	build-aux/git-version-gen \
	.scroom-version \
	.cproject \
	.project \
	.settings/org.eclipse.cdt.core.prefs

########################################################################
### Wrapping up
########################################################################

### Plugins
plugin_LTLIBRARIES = $(example_plugin_libs) $(transparent_overlay_plugin_libs) $(tiff_plugin_libs) $(colormap_plugin_libs) $(measure_plugin_libs)
plugins_check_programs = $(transparent_overlay_plugin_check_programs)
plugins_tests = $(transparent_overlay_plugin_tests)

### Libs
libs_libs = $(boost_test_helper_libs) $(sanity_tests_libs) $(util_libs) $(threadpool_libs) $(memory_manager_libs) $(scroomlib_libs) $(tiled_bitmap_libs)
libs_check_programs = $(boost_test_helper_check_programs) $(sanity_tests_check_programs) $(util_check_programs) $(threadpool_check_programs) $(memory_manager_check_programs) $(scroomlib_check_programs) $(scroomlib_gcheck_programs) $(tiled_bitmap_check_programs)
libs_tests = $(boost_test_helper_tests) $(sanity_tests_tests) $(util_tests) $(threadpool_tests) $(memory_manager_tests) $(scroomlib_tests) $(scroomlib_gtests) $(tiled_bitmap_tests)
libs_extra_dist = $(sanity_tests_extra_dist)
libs_noinst_libs = $(boost_test_helper_noinst_libs)

### Root ###############################################################
MOSTLYCLEANFILES=$(DX_CLEANFILES)

EXTRA_DIST = $(root_extra_dist) $(gui_extra_dist) $(libs_extra_dist)
pkgdata_DATA = $(gui_pkgdata_data)
bin_PROGRAMS = $(gui_bin_programs)
lib_LTLIBRARIES = $(libs_libs)
noinst_LTLIBRARIES = $(libs_noinst_libs)
check_PROGRAMS = $(libs_check_programs) $(plugins_check_programs) 
TESTS = $(libs_tests) $(plugins_tests) 

########################################################################
### Extra rules
########################################################################

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/gui/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/pixmaps; \
	  for pixmap in $(srcdir)/gui/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)$(pkgdatadir)/pixmaps; \
	    fi \
	  done \
	fi

dist-hook:
	if test -d gui/pixmaps; then \
	  mkdir $(distdir)/gui/pixmaps; \
	  for pixmap in gui/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/gui/pixmaps; \
	    fi \
	  done \
	fi

# Libraries need to be installed before plugins. Use a variable to
# avoid an automake error about custom rules overriding built-in ones.
plugin_lib_install_target = install-pluginLTLIBRARIES
$(plugin_lib_install_target): install-libLTLIBRARIES

# Target for building, but not executing, the tests
checkPROGRAMS: all-am $(check_PROGRAMS)

silent-doxygen:
	@$(MAKE) doxygen-doc > doxygen.log 2>&1

# Target for installing a portable version on Windows. Only does 'install' on Linux
install-win-portable: install
	@POST_INSTALL_HOOK@

	