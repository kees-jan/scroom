add_library(util assertions.cc counter.cc gtk-helpers.cc progressinterfacehelpers.cc)
target_link_libraries(
        util PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk pthreads Boost::system)
