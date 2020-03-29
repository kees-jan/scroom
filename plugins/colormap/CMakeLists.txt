add_library(spcolormap SHARED main.cc colormapplugin.cc colormapplugin.hh colormapprovider.cc colormapprovider.hh colormaps.cc colormaps.hh)
target_link_libraries(
        spcolormap PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(TARGETS spcolormap DESTINATION lib/scroom)

