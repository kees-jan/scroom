add_library(spmetadata SHARED)
target_sources(spmetadata PRIVATE main.cc metadata.cc metadata.hh)
target_link_libraries(
  spmetadata
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk threadpool PkgConfig::cairo)
install(TARGETS spmetadata DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(spmetadata)
