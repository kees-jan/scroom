add_library(sptiff SHARED main.cc tiff.cc tiff.hh tiffpresentation.cc tiffpresentation.hh)
target_link_libraries(
        sptiff PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk scroom_lib TIFF::TIFF tiledbitmap PkgConfig::cairo)

install(TARGETS sptiff DESTINATION lib/scroom)
