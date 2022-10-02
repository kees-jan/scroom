add_library(spcolormap SHARED)
target_sources(
  spcolormap
  PRIVATE main.cc
          colormapplugin.cc
          colormapplugin.hh
          colormapprovider.cc
          colormapprovider.hh
          colormaps.cc
          colormaps.hh
)
target_link_libraries(
  spcolormap
  PRIVATE project_options
          project_warnings
          spdlog
          plugin_interfaces
          PkgConfig::gtk
          PkgConfig::cairo
)
install(TARGETS spcolormap DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(spcolormap)
