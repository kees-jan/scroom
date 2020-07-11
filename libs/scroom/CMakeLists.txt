add_library(scroom_lib)
target_sources(scroom_lib PRIVATE
        src/bitmap-helpers.cc
        src/cairo-helpers.cc
        src/colormap-helpers.cc
        src/presentationinterface.cc
        src/transformpresentation.cc)
target_link_libraries(
        scroom_lib PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk PkgConfig::cairo)

install(TARGETS scroom_lib EXPORT scroom_lib INCLUDES DESTINATION include)
install(EXPORT scroom_lib DESTINATION lib/cmake NAMESPACE scroom)


if(ENABLE_BOOST_TEST)
    add_executable(scroom_lib_tests)
    target_sources(scroom_lib_tests PRIVATE test/main.cc test/colormaphelpers_test.cc)
    target_link_libraries(
            scroom_lib_tests PRIVATE boosttesthelper project_options project_warnings scroom_lib Boost::system)

    add_test(NAME scroom_lib_tests COMMAND scroom_lib_tests)
endif()

if(ENABLE_GOOGLE_TEST)
    add_executable(scroom_lib_gtests)
    target_sources(scroom_lib_gtests PRIVATE test/transformpresentation_test.cc)
    target_link_libraries(
            scroom_lib_gtests PRIVATE project_options project_warnings scroom_lib Boost::system CONAN_PKG::gtest)

    add_test(NAME scroom_lib_gtests COMMAND scroom_lib_gtests)
endif()
