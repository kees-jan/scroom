add_executable(transparentoverlay_tests main.cc determine-size-test.cc plugin-test.cc)
target_include_directories(transparentoverlay_tests PRIVATE ../src)
target_link_libraries(
        transparentoverlay_tests PRIVATE boosttesthelper project_options project_warnings sptransparentoverlay Boost::system)

add_test(NAME transparentoverlay_tests COMMAND transparentoverlay_tests)