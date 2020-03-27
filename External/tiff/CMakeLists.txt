find_package(TIFF REQUIRED)

if(TIFF_FOUND)
    set_target_properties(
            TIFF::TIFF
            PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
