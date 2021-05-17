find_package(PkgConfig REQUIRED)

pkg_check_modules(
  gtk
  REQUIRED
  IMPORTED_TARGET
  gtk+-3.0)
pkg_check_modules(
  cairo
  REQUIRED
  IMPORTED_TARGET
  cairo)
pkg_check_modules(
  gthread
  REQUIRED
  IMPORTED_TARGET
  gthread-2.0)
pkg_check_modules(
  gmodule
  REQUIRED
  IMPORTED_TARGET
  gmodule-2.0)

if(gtk_FOUND)
  set_target_properties(PkgConfig::gtk PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
if(cairo_FOUND)
  set_target_properties(PkgConfig::cairo PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
if(gthread_FOUND)
  set_target_properties(PkgConfig::gthread PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
if(gmodule_FOUND)
  set_target_properties(PkgConfig::gmodule PROPERTIES IMPORTED_GLOBAL TRUE)
endif()
