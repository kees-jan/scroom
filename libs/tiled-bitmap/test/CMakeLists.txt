add_executable(tiledbitmap_tests main.cc tiledbitmap-tests.cc)
target_link_libraries(
        tiledbitmap_tests PRIVATE boosttesthelper project_options project_warnings tiledbitmap Boost::system)

add_test(NAME tiledbitmap_tests COMMAND tiledbitmap_tests)