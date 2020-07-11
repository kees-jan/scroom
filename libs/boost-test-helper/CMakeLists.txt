if(ENABLE_BOOST_TEST)
  add_library(boosttesthelper STATIC)
  target_sources(boosttesthelper PRIVATE src/unittest-main.cc)
  target_link_libraries(
    boosttesthelper
    PRIVATE project_options project_warnings
    PUBLIC Boost::unit_test_framework)

  add_executable(boosttesthelper_tests)
  target_sources(boosttesthelper_tests PRIVATE test/main.cc test/testlib-tests.cc)
  target_link_libraries(boosttesthelper_tests PRIVATE boosttesthelper project_options project_warnings)

  add_test(NAME boosttesthelper_tests COMMAND boosttesthelper_tests)
endif()
