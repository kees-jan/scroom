add_subdirectory(src)

if(ENABLE_BOOST_TEST)
  add_executable(ruler_tests)
  target_sources(ruler_tests PRIVATE test/main.cc test/ruler-tests.cc src/)
  target_include_directories(ruler_tests PRIVATE src)
  target_link_libraries(
    ruler_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            ruler_lib)

  add_test(NAME ruler_tests COMMAND ruler_tests)
endif()

set(abs_top_builddir "${Scroom_BINARY_DIR}")
configure_file(ENVIRONMENT.in ${Scroom_BINARY_DIR}/ENVIRONMENT @ONLY)

install(FILES scroom.glade DESTINATION ${PKG_DIR_RELATIVE})
