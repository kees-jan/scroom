#include "pipette.hh"

#include <gdk/gdk.h>

////////////////////////////////////////////////////////////////////////
// Pipette
////////////////////////////////////////////////////////////////////////

Pipette::Pipette()
{
  selection = nullptr;
  enabled = false;
}
Pipette::~Pipette()
{
}

Pipette::Ptr Pipette::create()
{
  return Ptr(new Pipette());
}

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Pipette::getPluginName()
{
  return "Pipette";
}

std::string Pipette::getPluginVersion()
{
  return "0.0";
}

void Pipette::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerViewObserver("Pipette", shared_from_this<Pipette>());
}

////////////////////////////////////////////////////////////////////////
// ViewObserver
////////////////////////////////////////////////////////////////////////

static void on_toggled(GtkToggleButton* button, gpointer data)
{
  bool* view = reinterpret_cast<bool*>(data);
  *view = !gtk_toggle_button_get_active(button);
}

Scroom::Bookkeeping::Token Pipette::viewAdded(ViewInterface::Ptr v)
{
  gdk_threads_enter();

  view = v;
  view->registerSelectionListener(shared_from_this<Pipette>(), MouseButton::PRIMARY);
  view->registerPostRenderer(shared_from_this<Pipette>());

  GtkToolItem* button = gtk_tool_item_new();
  GtkWidget* toggleButton = gtk_toggle_button_new_with_mnemonic("pipette");
  gtk_widget_set_visible(toggleButton, true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggleButton), true);

  gtk_container_add(GTK_CONTAINER(button), toggleButton);
  g_signal_connect(static_cast<gpointer>(toggleButton), "toggled", G_CALLBACK(on_toggled), &enabled);

  view->addToToolbar(button);

  gdk_threads_leave();

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void Pipette::onSelectionStart(GdkPoint)
{
  if(enabled)
  {
    view->unsetPanning();
  }
}

void Pipette::onSelectionUpdate(Selection* s)
{
  if(enabled)
  {
    selection = s;
  }
}

void Pipette::onSelectionEnd(Selection* s)
{
  if(enabled)
  {
    selection = s;
    //TODO show data?
    view->setPanning();
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void Pipette::render(cairo_t* cr)
{
  if(selection && enabled)
  {
    GdkPoint start = view->presentationPointToWindowPoint(selection->start);
    GdkPoint end = view->presentationPointToWindowPoint(selection->end);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 0, 0, 1); // Blue
    cairo_move_to(cr, end.x, start.y);
    cairo_line_to(cr, start.x, start.y);
    cairo_line_to(cr, start.x, end.y);
    cairo_line_to(cr, end.x, end.y);
    cairo_line_to(cr, end.x, start.y);
    cairo_stroke(cr);
  }
}
