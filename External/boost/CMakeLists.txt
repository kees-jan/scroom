find_package(
  Boost REQUIRED
  COMPONENTS thread
             system
             unit_test_framework
             filesystem
             chrono)

if(Boost_FOUND)
  set_target_properties(
    Boost::boost
    Boost::system
    Boost::thread
    Boost::unit_test_framework
    Boost::filesystem
    Boost::chrono
    PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
