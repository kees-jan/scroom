add_library(util)
set(HEADER_FILES
    inc/scroom/assertions.hh
    inc/scroom/bookkeeping.hh
    inc/scroom/dont-delete.hh
    inc/scroom/format_stuff.hh
    inc/scroom/gtk-helpers.hh
    inc/scroom/gtk-test-helpers.hh
    inc/scroom/interface.hh
    inc/scroom/linearsegment.hh
    inc/scroom/observable.hh
    inc/scroom/point.hh
    inc/scroom/progressinterface.hh
    inc/scroom/progressinterfacehelpers.hh
    inc/scroom/rectangle.hh
    inc/scroom/rounding.hh
    inc/scroom/stuff.hh
    inc/scroom/utilities.hh
)
set(HEADER_FILES_IMPL inc/scroom/impl/bookkeepingimpl.hh)
target_sources(
  util
  PRIVATE src/assertions.cc
          src/counter.cc
          src/gtk-helpers.cc
          src/gtk-test-helpers.cc
          src/progressinterfacehelpers.cc
          ${HEADER_FILES}
          ${HEADER_FILES_IMPL}
)
target_link_libraries(
  util
  PRIVATE project_options
          project_warnings
          spdlog
          PkgConfig::gtk
  PUBLIC PkgConfig::gtk Boost::system
)
target_include_directories(util PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include>)

install(
  TARGETS util
  EXPORT util
  INCLUDES
  DESTINATION include
)
install(
  EXPORT util
  DESTINATION lib/cmake
  NAMESPACE scroom
)
install(FILES ${HEADER_FILES} DESTINATION include/scroom)
install(FILES ${HEADER_FILES_IMPL} DESTINATION include/scroom/impl)

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
            test/scope-exit-tests.cpp
            test/rectangletests.cc
  )
  target_link_libraries(
    util_tests
    PRIVATE boosttesthelper
            project_options
            project_warnings
            plugin_interfaces
  )

  add_test(NAME util_tests COMMAND util_tests)
endif()
