#include <gtk/gtk.h>

#include <scroom/gtk-helpers.hh>
#include <scroom/gtk-test-helpers.hh>

namespace Scroom::GtkTestHelpers
{
  GtkMainLoop::GtkMainLoop()
    : gtk_thread(GtkMainLoop::run_gtk)
  {}

  GtkMainLoop::~GtkMainLoop()
  {
    Scroom::GtkHelpers::async_on_ui_thread(gtk_main_quit);
    gtk_thread.join();
  }

  void GtkMainLoop::run_gtk()
  {
    gtk_init(0, NULL);
    gtk_main();
  }


} // namespace Scroom::GtkTestHelpers
