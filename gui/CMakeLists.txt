add_subdirectory(src)

set(abs_top_builddir "${Scroom_BINARY_DIR}")
configure_file(ENVIRONMENT.in ${Scroom_BINARY_DIR}/ENVIRONMENT @ONLY)
