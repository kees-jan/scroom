find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
  set_target_properties(ZLIB::ZLIB PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
