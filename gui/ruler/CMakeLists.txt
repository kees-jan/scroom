add_library(ruler_lib SHARED)
target_sources(
  ruler_lib
  PRIVATE src/ruler.cc
          src/ruler.hh
          src/rulerstrategies.cc
          src/rulerstrategies.hh)
target_link_libraries(
  ruler_lib
  PRIVATE project_options project_warnings
  PUBLIC util PkgConfig::gtk Boost::system)

if(ENABLE_BOOST_TEST)
  add_executable(ruler_tests)
  target_sources(ruler_tests PRIVATE test/main.cc test/ruler-tests.cc)
  target_include_directories(ruler_tests PRIVATE src)
  target_link_libraries(
    ruler_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            ruler_lib)

  add_test(NAME ruler_tests COMMAND ruler_tests)
endif()
