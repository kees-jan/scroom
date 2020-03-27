find_package(Boost REQUIRED COMPONENTS thread system unit_test_framework)

if(Boost_FOUND)
    set_target_properties(
            Boost::boost
            Boost::system
            Boost::thread
            Boost::unit_test_framework
            PROPERTIES IMPORTED_GLOBAL TRUE)
endif()