add_library(ruler SHARED)
set(HEADER_FILES inc/ruler.hh inc/rulerstrategies.hh)
target_sources(ruler PRIVATE src/ruler.cc src/rulerstrategies.cc ${HEADER_FILES})
target_link_libraries(
  ruler
  PRIVATE project_options project_warnings
  PUBLIC PkgConfig::gtk util)
target_include_directories(ruler PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>)
install(
  TARGETS ruler
  EXPORT ruler
  INCLUDES
  DESTINATION include)
install(
  EXPORT ruler
  DESTINATION lib/cmake
  NAMESPACE scroom)
install(FILES ${HEADER_FILES} DESTINATION include)

if(ENABLE_BOOST_TEST)
  add_executable(ruler_tests)
  target_sources(ruler_tests PRIVATE test/main.cc test/ruler-tests.cc)
  target_include_directories(ruler_tests PRIVATE src)
  target_link_libraries(
    ruler_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            ruler)

  add_test(NAME ruler_tests COMMAND ruler_tests)
endif()
