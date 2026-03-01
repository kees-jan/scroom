set(Boost_USE_STATIC_LIBS OFF)
find_package(
  Boost CONFIG REQUIRED
  COMPONENTS thread
             system
             filesystem
             chrono
             program_options
)

if(Boost_FOUND)
  set_target_properties(
    Boost::boost
    Boost::system
    Boost::thread
    Boost::filesystem
    Boost::chrono
    Boost::program_options
    PROPERTIES IMPORTED_GLOBAL TRUE
  )
endif()
