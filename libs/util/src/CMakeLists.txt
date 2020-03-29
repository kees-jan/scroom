add_library(util assertions.cc counter.cc gtk-helpers.cc progressinterfacehelpers.cc)
target_link_libraries(
        util PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk pthreads Boost::system)

install(TARGETS util EXPORT util INCLUDES DESTINATION include)
install(EXPORT util DESTINATION lib/cmake NAMESPACE scroom)