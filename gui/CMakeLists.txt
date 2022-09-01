add_subdirectory(ruler)
add_subdirectory(src)

set(abs_top_builddir "${SCROOM_BUILD_OUTPUT_DIR}")
configure_file(ENVIRONMENT.in ${SCROOM_BUILD_OUTPUT_DIR}/ENVIRONMENT @ONLY)
configure_file(ENVIRONMENT.cmd.in ${SCROOM_BUILD_OUTPUT_DIR}/ENVIRONMENT.cmd @ONLY)

install(FILES scroom.glade DESTINATION ${PKG_DIR_RELATIVE})
