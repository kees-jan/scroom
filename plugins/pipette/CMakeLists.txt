add_library(sppipette SHARED)
target_sources(sppipette PRIVATE src/main.cc src/pipette.cc src/pipette.hh)
target_link_libraries(
  sppipette
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk threadpool PkgConfig::cairo)

install(TARGETS sppipette DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})

if(ENABLE_BOOST_TEST)
  add_executable(sppipette_tests)
  target_sources(sppipette_tests PRIVATE test/main.cc test/pipette-tests.cc)
  target_link_libraries(
    sppipette_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            sppipette)
  target_include_directories(sppipette_tests PRIVATE src)

  add_test(NAME sppipette_tests COMMAND sppipette_tests)
endif()
