add_library(sppipette SHARED)
target_sources(sppipette PRIVATE src/main.cc src/pipette.cc src/pipette.hh)
target_link_libraries(
  sppipette
  PRIVATE project_options
          project_warnings
          scroom_lib
          spdlog
  PUBLIC PkgConfig::gtk
         threadpool
         PkgConfig::cairo
         plugin_interfaces
)

install(TARGETS sppipette DESTINATION ${PLUGIN_INSTALL_LOCATION_RELATIVE})
install_plugin_dependencies(sppipette)

if(ENABLE_GOOGLE_TEST)
  add_executable(sppipette_tests)
  target_sources(sppipette_tests PRIVATE test/pipette-tests.cc)
  target_include_directories(sppipette_tests PRIVATE src)
  target_link_libraries(
    sppipette_tests
    PRIVATE gmock_main
            project_options
            project_warnings
            sppipette
            util
  )

  add_test(NAME sppipette_tests COMMAND sppipette_tests)
endif()
