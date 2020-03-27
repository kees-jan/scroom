
if(ENABLE_BOOST_TEST)
    add_executable(scroom_lib_tests main.cc colormaphelpers_test.cc)
    target_link_libraries(
            scroom_lib_tests PRIVATE boosttesthelper project_options project_warnings scroom_lib Boost::system)

    add_test(NAME scroom_lib_tests COMMAND scroom_lib_tests)
endif()

if(ENABLE_GOOGLE_TEST)
    add_executable(scroom_lib_gtests transformpresentation_test.cc)
    target_link_libraries(
            scroom_lib_gtests PRIVATE project_options project_warnings scroom_lib Boost::system CONAN_PKG::gtest)

    add_test(NAME scroom_lib_gtests COMMAND scroom_lib_gtests)
endif()
