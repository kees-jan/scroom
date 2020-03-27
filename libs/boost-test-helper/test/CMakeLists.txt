add_executable(boosttesthelper_tests main.cc testlib-tests.cc)
target_link_libraries(
        boosttesthelper_tests PRIVATE boosttesthelper project_options project_warnings)

add_test(NAME boosttesthelper_tests COMMAND boosttesthelper_tests)