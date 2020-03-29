add_library(tiledbitmap compressedtile.cc layer.cc layercoordinator.cc layercoordinator.hh layeroperations.cc local.hh tiled-bitmap.cc tiled-bitmap.hh tiledbitmapviewdata.cc tiledbitmapviewdata.hh tileviewstate.cc tileviewstate.hh)
target_link_libraries(
        tiledbitmap PRIVATE project_options project_warnings util threadpool memory_manager scroom_lib PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(TARGETS tiledbitmap EXPORT tiledbitmap INCLUDES DESTINATION include)
install(EXPORT tiledbitmap DESTINATION lib/cmake NAMESPACE scroom)
