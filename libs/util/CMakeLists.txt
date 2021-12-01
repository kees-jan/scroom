add_library(util)
target_sources(
  util
  PRIVATE src/assertions.cc
          src/counter.cc
          src/gtk-helpers.cc
          src/gtk-test-helpers.cc
          src/progressinterfacehelpers.cc)
target_link_libraries(
  util
  PRIVATE project_options project_warnings CONAN_PKG::spdlog
  PUBLIC PkgConfig::gtk pthreads Boost::system)

install(
  TARGETS util
  EXPORT util
  INCLUDES
  DESTINATION include)
install(
  EXPORT util
  DESTINATION lib/cmake
  NAMESPACE scroom)

if(ENABLE_BOOST_TEST)
  add_executable(util_tests)
  target_sources(
    util_tests
    PRIVATE test/bookkeeping-tests.cc
            test/counter-tests.cc
            test/gtkhelper-tests.cc
            test/main.cc
            test/observable-tests.cc
            test/progressinterfacebroadcaster-tests.cc
            test/progressinterfaceconversion-tests.cc
            test/progressinterfacemultiplexer-tests.cc
            test/progressstateinterfacestub.cc
            test/progressstateinterfacestub.hh
            test/scope-exit-tests.cpp)
  target_link_libraries(
    util_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            util)

  add_test(NAME util_tests COMMAND util_tests)
endif()
