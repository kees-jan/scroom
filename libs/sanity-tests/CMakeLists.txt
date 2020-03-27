add_executable(sanity_tests main.cc basic-assumptions.cc boost-bind.cc boost-smart-pointers.cc)
target_link_libraries(
        sanity_tests PRIVATE boosttesthelper project_options project_warnings)

add_test(NAME sanity_tests COMMAND sanity_tests)
