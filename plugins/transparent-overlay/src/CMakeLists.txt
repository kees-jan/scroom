add_library(sptransparentoverlay SHARED main.cc sizedeterminer.cc sizedeterminer.hh transparentoverlay.cc transparentoverlay.hh transparentoverlaypresentation.cc transparentoverlaypresentation.hh transparentoverlayviewinfo.cc transparentoverlayviewinfo.hh)
target_link_libraries(
        sptransparentoverlay PRIVATE project_options project_warnings PUBLIC PkgConfig::gtk scroom_lib util)

install(TARGETS sptransparentoverlay DESTINATION lib/scroom)
