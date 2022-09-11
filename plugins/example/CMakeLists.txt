add_library(spexample SHARED)
target_sources(
  spexample
  PRIVATE main.cc
          example.cc
          example.hh
          examplepresentation.cc
          examplepresentation.hh)
target_link_libraries(
  spexample
  PRIVATE project_options
          project_warnings
          scroom_lib
          plugin_interfaces
          PkgConfig::gtk)
