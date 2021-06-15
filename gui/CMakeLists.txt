add_subdirectory(ruler)
add_subdirectory(src)

set(abs_top_builddir "${Scroom_BINARY_DIR}")
configure_file(ENVIRONMENT.in ${Scroom_BINARY_DIR}/ENVIRONMENT @ONLY)

install(FILES scroom.glade DESTINATION ${PKG_DIR_RELATIVE})
