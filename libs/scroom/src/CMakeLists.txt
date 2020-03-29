add_library(scroom_lib bitmap-helpers.cc cairo-helpers.cc colormap-helpers.cc presentationinterface.cc transformpresentation.cc)
target_link_libraries(
        scroom_lib PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(TARGETS scroom_lib EXPORT scroom_lib INCLUDES DESTINATION include)
install(EXPORT scroom_lib DESTINATION lib/cmake NAMESPACE scroom)
