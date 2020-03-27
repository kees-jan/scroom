add_library(gui SHARED main.cc callbacks.cc callbacks.hh loader.cc loader.hh pluginmanager.cc pluginmanager.hh progressbarmanager.cc progressbarmanager.hh sidebarmanager.cc sidebarmanager.hh view.cc view.hh workinterface.hh)
target_link_libraries(
        gui PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk PkgConfig::cairo PkgConfig::glade PkgConfig::gthread PkgConfig::gmodule)
target_compile_definitions(gui PRIVATE
        PLUGIN_DIR="Bla"
        TOP_SRCDIR="Bloe"
        PACKAGE_DATA_DIR="eh"
        PACKAGE_NAME="Scroom"
        PACKAGE_VERSION="1.0")