if(ENABLE_BOOST_TEST)
  add_executable(sanity_tests)
  target_sources(
    sanity_tests
    PRIVATE main.cc
            basic-assumptions.cc
            boost-bind.cc
            boost-smart-pointers.cc)
  target_link_libraries(sanity_tests PRIVATE boosttesthelper project_options project_warnings)

  add_test(NAME sanity_tests COMMAND sanity_tests)
endif()
