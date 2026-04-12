set(Boost_USE_STATIC_LIBS OFF)
find_package(
  Boost CONFIG REQUIRED
  COMPONENTS thread
             filesystem
             chrono
             program_options
)

if(Boost_FOUND)
  set_target_properties(
    Boost::headers
    Boost::thread
    Boost::filesystem
    Boost::chrono
    Boost::program_options
    PROPERTIES IMPORTED_GLOBAL TRUE
  )
endif()
