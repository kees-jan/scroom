add_library(sptransparentoverlay SHARED)
target_sources(
  sptransparentoverlay
  PRIVATE src/main.cc
          src/sizedeterminer.cc
          src/sizedeterminer.hh
          src/transparentoverlay.cc
          src/transparentoverlay.hh
          src/transparentoverlaypresentation.cc
          src/transparentoverlaypresentation.hh
          src/transparentoverlayviewinfo.cc
          src/transparentoverlayviewinfo.hh)
target_link_libraries(
  sptransparentoverlay
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk scroom_lib util)

install(TARGETS sptransparentoverlay DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})

if(ENABLE_BOOST_TEST)
  add_executable(transparentoverlay_tests)
  target_sources(transparentoverlay_tests PRIVATE test/main.cc test/determine-size-test.cc test/plugin-test.cc)
  target_include_directories(transparentoverlay_tests PRIVATE src)
  target_link_libraries(
    transparentoverlay_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            sptransparentoverlay
            Boost::system)

  add_test(NAME transparentoverlay_tests COMMAND transparentoverlay_tests)
endif()
