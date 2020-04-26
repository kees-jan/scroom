add_executable(scroom main.cc callbacks.cc callbacks.hh loader.cc loader.hh pluginmanager.cc pluginmanager.hh progressbarmanager.cc progressbarmanager.hh sidebarmanager.cc sidebarmanager.hh view.cc view.hh workinterface.hh)
target_link_libraries(
        scroom PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk PkgConfig::cairo PkgConfig::glade PkgConfig::gthread PkgConfig::gmodule Boost::system Boost::filesystem util)
target_compile_definitions(scroom PRIVATE
        PLUGIN_DIR="Bla"
        TOP_SRCDIR="Bloe"
        PACKAGE_DATA_DIR="eh")

install(TARGETS scroom)
