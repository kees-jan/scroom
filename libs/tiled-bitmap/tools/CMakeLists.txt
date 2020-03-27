
add_library(measure_tools OBJECT
        test-helpers.hh test-helpers.cc
        measure-framerate-callbacks.cc measure-framerate-callbacks.hh
        measure-framerate-stubs.cc measure-framerate-stubs.hh)
target_link_libraries(measure_tools PUBLIC project_options project_warnings tiledbitmap)

add_executable(measure_framerate measure-framerate.cc measure-framerate-tests.cc measure-framerate-tests.hh)
target_link_libraries(measure_framerate PRIVATE project_options project_warnings measure_tools Boost::system PkgConfig::gthread)

add_executable(measure_load_performance measure-load-performance-tests.cc measure-load-performance-tests.hh measure-load-performance.cc)
target_link_libraries(measure_load_performance PRIVATE project_options project_warnings measure_tools threadpool PkgConfig::gthread)
