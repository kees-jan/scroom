add_library(tiledbitmap src/tiledbitmappresentation.cc)
target_sources(
  tiledbitmap
  PRIVATE src/cmyklayeroperations.cc
          src/compressedtile.cc
          src/layer.cc
          src/layercoordinator.cc
          src/layercoordinator.hh
          src/layeroperations.cc
          src/layerspecforbitmap.cc
          src/local.hh
          src/tiled-bitmap.cc
          src/tiled-bitmap.hh
          src/tiledbitmapviewdata.cc
          src/tiledbitmapviewdata.hh
          src/tileviewstate.cc
          src/tileviewstate.hh)
target_link_libraries(
  tiledbitmap
  PRIVATE project_options
          project_warnings
          util
          threadpool
          memory_manager
          scroom_lib
  PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(
  TARGETS tiledbitmap
  EXPORT tiledbitmap
  INCLUDES
  DESTINATION include)
install(
  EXPORT tiledbitmap
  DESTINATION lib/cmake
  NAMESPACE scroom)

add_library(measure_tools OBJECT)
target_sources(
  measure_tools
  PRIVATE tools/test-helpers.hh
          tools/test-helpers.cc
          tools/measure-framerate-callbacks.cc
          tools/measure-framerate-callbacks.hh
          tools/measure-framerate-stubs.cc
          tools/measure-framerate-stubs.hh)
target_link_libraries(measure_tools PUBLIC project_options project_warnings tiledbitmap)

add_executable(measure_framerate)
target_sources(measure_framerate PRIVATE tools/measure-framerate.cc tools/measure-framerate-tests.cc
                                         tools/measure-framerate-tests.hh)
target_link_libraries(
  measure_framerate
  PRIVATE project_options
          project_warnings
          measure_tools
          Boost::system
          PkgConfig::gthread)

add_executable(measure_load_performance)
target_sources(
  measure_load_performance PRIVATE tools/measure-load-performance-tests.cc tools/measure-load-performance-tests.hh
                                   tools/measure-load-performance.cc)
target_link_libraries(
  measure_load_performance
  PRIVATE project_options
          project_warnings
          measure_tools
          threadpool
          PkgConfig::gthread)

if(ENABLE_BOOST_TEST)
  add_executable(tiledbitmap_tests)
  target_sources(tiledbitmap_tests PRIVATE test/main.cc test/tiledbitmap-tests.cc test/sampleiterator-tests.cc)
  target_link_libraries(
    tiledbitmap_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            tiledbitmap
            Boost::system)

  add_test(NAME tiledbitmap_tests COMMAND tiledbitmap_tests)
endif()
