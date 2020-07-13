add_library(sptiff SHARED)
target_sources(
  sptiff
  PRIVATE src/main.cc
          src/tiff.cc
          src/tiff.hh
          src/tiffpresentation.cc
          src/tiffpresentation.hh)
target_link_libraries(
  sptiff
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk
         scroom_lib
         util
         TIFF::TIFF
         tiledbitmap
         PkgConfig::cairo)

install(TARGETS sptiff DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(sptiff)
