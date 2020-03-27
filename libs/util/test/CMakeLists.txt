add_executable(util_tests main.cc bookkeeping-tests.cc counter-tests.cc gtkhelper-tests.cc observable-tests.cc progressinterfacebroadcaster-tests.cc progressinterfaceconversion-tests.cc progressinterfacemultiplexer-tests.cc progressstateinterfacestub.cc progressstateinterfacestub.hh)
target_link_libraries(
        util_tests PRIVATE boosttesthelper project_options project_warnings util)

add_test(NAME util_tests COMMAND util_tests)