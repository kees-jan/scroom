add_subdirectory(src)
add_subdirectory(tools)

if(ENABLE_BOOST_TEST)
    add_subdirectory(test)
endif()
