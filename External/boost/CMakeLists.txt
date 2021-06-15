find_package(
  Boost REQUIRED
  COMPONENTS thread
             system
             unit_test_framework
             filesystem
             chrono
             program_options)

if(Boost_FOUND)
  set_target_properties(
    Boost::boost
    Boost::system
    Boost::thread
    Boost::unit_test_framework
    Boost::filesystem
    Boost::chrono
    Boost::program_options
    PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
