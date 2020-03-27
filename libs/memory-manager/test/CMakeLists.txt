add_executable(memory_manager_tests main.cc blob-tests.cc compression-tests.cc pageprovider-tests.cc swap-block-allocator-tests.cc)
target_link_libraries(
        memory_manager_tests PRIVATE boosttesthelper project_options project_warnings memory_manager Boost::system)
target_include_directories(memory_manager_tests PRIVATE ../src)

add_test(NAME memory_manager_tests COMMAND memory_manager_tests)