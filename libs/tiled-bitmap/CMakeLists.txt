add_library(tiledbitmap src/tiledbitmappresentation.cc)
set(HEADER_FILES
    inc/scroom/layeroperations.hh
    inc/scroom/tile.hh
    inc/scroom/tiledbitmapinterface.hh
    inc/scroom/tiledbitmaplayer.hh
)
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
          src/tileviewstate.hh
          ${HEADER_FILES}
)
target_link_libraries(
  tiledbitmap
  PRIVATE project_options
          project_warnings
          threadpool
          memory_manager
          scroom_lib
          spdlog
          fmt
  PUBLIC PkgConfig::gtk
         PkgConfig::cairo
         memory_manager
         threadpool
         plugin_interfaces
)
target_include_directories(
  tiledbitmap PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>
)

install(
  TARGETS tiledbitmap
  EXPORT tiledbitmap
  INCLUDES
  DESTINATION include
)
install(
  EXPORT tiledbitmap
  DESTINATION lib/cmake
  NAMESPACE scroom
)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)

add_library(measure_tools OBJECT)
target_sources(
  measure_tools
  PRIVATE tools/test-helpers.hh
          tools/test-helpers.cc
          tools/measure-framerate-callbacks.cc
          tools/measure-framerate-callbacks.hh
          tools/measure-framerate-stubs.cc
          tools/measure-framerate-stubs.hh
)
target_link_libraries(
  measure_tools
  PUBLIC project_options
         project_warnings
         tiledbitmap
         scroom_lib
)

add_executable(measure_framerate)
target_sources(
  measure_framerate PRIVATE tools/measure-framerate.cc tools/measure-framerate-tests.cc
                            tools/measure-framerate-tests.hh
)
target_link_libraries(
  measure_framerate
  PRIVATE project_options
          project_warnings
          measure_tools
          Boost::system
          PkgConfig::gthread
          spdlog
)

add_executable(measure_load_performance)
target_sources(
  measure_load_performance PRIVATE tools/measure-load-performance-tests.cc tools/measure-load-performance-tests.hh
                                   tools/measure-load-performance.cc
)
target_link_libraries(
  measure_load_performance
  PRIVATE project_options
          project_warnings
          measure_tools
          threadpool
          PkgConfig::gthread
          spdlog
)

if(ENABLE_BOOST_TEST)
  add_executable(tiledbitmap_tests)
  target_sources(tiledbitmap_tests PRIVATE test/main.cc test/tiledbitmap-tests.cc test/sampleiterator-tests.cc)
  target_link_libraries(
    tiledbitmap_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            tiledbitmap
            Boost::system
            scroom_lib
  )

  add_test(NAME tiledbitmap_tests COMMAND tiledbitmap_tests)
endif()
