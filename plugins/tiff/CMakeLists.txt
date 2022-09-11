add_library(sptiff SHARED)
target_sources(
  sptiff
  PRIVATE src/main.cc
          src/tiff.cc
          src/tiff.hh
          src/tiffsource.hh
          src/tiffsource.cc)
target_link_libraries(
  sptiff
  PRIVATE project_options
          project_warnings
          spdlog
          PkgConfig::gtk
          scroom_lib
          TIFF::TIFF
          tiledbitmap
          PkgConfig::cairo)

install(TARGETS sptiff DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(sptiff)
