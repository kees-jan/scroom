if (ENABLE_BOOST_TEST)
    add_library(boosttesthelper STATIC src/unittest-main.cc)
    target_link_libraries(
            boosttesthelper PRIVATE project_options project_warnings PUBLIC Boost::unit_test_framework)

    add_executable(boosttesthelper_tests test/main.cc test/testlib-tests.cc)
    target_link_libraries(
            boosttesthelper_tests PRIVATE boosttesthelper project_options project_warnings)

    add_test(NAME boosttesthelper_tests COMMAND boosttesthelper_tests)
endif ()
