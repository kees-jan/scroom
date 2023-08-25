add_library(scroom_lib)
set(HEADER_FILES
    inc/scroom/bitmap-helpers.hh
    inc/scroom/cairo-helpers.hh
    inc/scroom/global.hh
    inc/scroom/showmetadata.hh
    inc/scroom/transformpresentation.hh
    inc/scroom/tweak-view.hh
)
target_sources(
  scroom_lib
  PRIVATE src/bitmap-helpers.cc
          src/cairo-helpers.cc
          src/colormap-helpers.cc
          src/showmetadata.cc
          src/presentationinterface.cc
          src/transformpresentation.cc
          src/tweak-view.cc
          ${HEADER_FILES}
)
target_link_libraries(
  scroom_lib
  PRIVATE project_options
          project_warnings
          fmt
          plugin_interfaces
          PkgConfig::gtk
          PkgConfig::cairo
)

target_include_directories(
  scroom_lib PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>
)

install(
  TARGETS scroom_lib
  EXPORT scroom_lib
  INCLUDES
  DESTINATION include
)
install(
  EXPORT scroom_lib
  DESTINATION lib/cmake
  NAMESPACE scroom
)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)

if(ENABLE_BOOST_TEST)
  add_executable(scroom_lib_tests)
  target_sources(scroom_lib_tests PRIVATE test/main.cc test/colormaphelpers_test.cc)
  target_link_libraries(
    scroom_lib_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            scroom_lib
            Boost::system
            plugin_interfaces
  )

  add_test(NAME scroom_lib_tests COMMAND scroom_lib_tests)
endif()

if(ENABLE_GOOGLE_TEST)
  add_executable(scroom_lib_gtests)
  target_sources(scroom_lib_gtests PRIVATE test/transformpresentation_test.cc)
  target_link_libraries(
    scroom_lib_gtests
    PRIVATE project_options
            project_warnings
            scroom_lib
            Boost::system
            gmock_main
            plugin_interfaces
  )

  add_test(NAME scroom_lib_gtests COMMAND scroom_lib_gtests)
endif()
