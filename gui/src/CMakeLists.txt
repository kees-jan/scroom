add_executable(scroom WIN32)
target_sources(
  scroom
  PRIVATE main.cc
          callbacks.cc
          callbacks.hh
          loader.cc
          loader.hh
          pluginmanager.cc
          pluginmanager.hh
          progressbarmanager.cc
          progressbarmanager.hh
          sidebarmanager.cc
          sidebarmanager.hh
          view.cc
          view.hh
          workinterface.hh
          ruler.cc
          ruler.hh)

if(WIN32)
  target_sources(scroom PRIVATE ../icon/icon.rc)
endif()

target_link_libraries(
  scroom
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk
         PkgConfig::cairo
         PkgConfig::gthread
         PkgConfig::gmodule
         Boost::system
         Boost::filesystem
         util
         tiledbitmap
         m)
target_compile_definitions(scroom PRIVATE PLUGIN_DIR="${PLUGIN_INSTALL_LOCATION_ABSOLUTE}"
                                          TOP_SRCDIR="${Scroom_SOURCE_DIR}" PACKAGE_DATA_DIR="${PKG_DIR_ABSOLUTE}")

install(TARGETS scroom)
