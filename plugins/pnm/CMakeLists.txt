add_library(sppnm SHARED)
target_sources(
  sppnm
  PRIVATE src/main.cc
          src/pnm.cc
          src/pnm.hh
          src/pnmsource.hh
          src/pnmsource.cc
)
target_link_libraries(
  sppnm
  PRIVATE project_options
          project_warnings
          spdlog
          PkgConfig::gtk
          scroom_lib
          tiledbitmap
          PkgConfig::cairo
)

install(TARGETS sppnm DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(sppnm)
