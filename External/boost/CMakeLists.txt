find_package(
  Boost REQUIRED
  COMPONENTS thread
             system
             unit_test_framework
             filesystem)

if(Boost_FOUND)
  set_target_properties(
    Boost::boost
    Boost::system
    Boost::thread
    Boost::unit_test_framework
    Boost::filesystem
    PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
