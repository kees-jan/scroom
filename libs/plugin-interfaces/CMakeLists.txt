add_library(plugin_interfaces INTERFACE)
set(HEADER_FILES
    inc/scroom/color.hh
    inc/scroom/colormappable.hh
    inc/scroom/opentiledbitmapinterface.hh
    inc/scroom/pipettelayeroperations.hh
    inc/scroom/plugininformationinterface.hh
    inc/scroom/presentationinterface.hh
    inc/scroom/resizablepresentationinterface.hh
    inc/scroom/scroominterface.hh
    inc/scroom/scroomplugin.hh
    inc/scroom/showmetadatainterface.hh
    inc/scroom/viewinterface.hh)
set(HEADER_FILES_GMOCK inc/scroom/gmock/colormappable.hh inc/scroom/gmock/presentationinterface.hh)
target_sources(plugin_interfaces PRIVATE ${HEADER_FILES} ${HEADER_FILES_GMOCK})

target_include_directories(plugin_interfaces INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
                                                       $<INSTALL_INTERFACE:include>)
target_link_libraries(plugin_interfaces INTERFACE util)

install(
  TARGETS plugin_interfaces
  EXPORT plugin_interfaces
  INCLUDES
  DESTINATION include)
install(
  EXPORT plugin_interfaces
  DESTINATION lib/cmake
  NAMESPACE scroom)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)
install(FILES ${HEADER_FILES_GMOCK} DESTINATION include/scroom/gmock)
