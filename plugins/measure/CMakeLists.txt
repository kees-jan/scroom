add_library(spmeasure SHARED)
target_sources(spmeasure PRIVATE src/main.cc src/measure.cc src/measure.hh)
target_link_libraries(
  spmeasure
  PRIVATE project_options
          project_warnings
          scroom_lib
          fmt
  PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(TARGETS spmeasure DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(spmeasure)
