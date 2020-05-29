#include "pipette.hh"

#include <gdk/gdk.h>
#include <cmath>
#include <scroom/pipetteviewinterface.hh>

////////////////////////////////////////////////////////////////////////
// Pipette
////////////////////////////////////////////////////////////////////////

Pipette::Pipette()
{
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
  bool* flag = reinterpret_cast<bool*>(data);
  *flag = gtk_toggle_button_get_active(button);
}

Scroom::Bookkeeping::Token Pipette::viewAdded(ViewInterface::Ptr v)
{
  gdk_threads_enter();

  PipetteHandler::Ptr handler = PipetteHandler::create();
  handler->view = v;
  v->registerSelectionListener(handler, MouseButton::PRIMARY);
  v->registerPostRenderer(handler);

  GtkToolItem* button = gtk_tool_item_new();
  GtkWidget* toggleButton = gtk_toggle_button_new_with_mnemonic("pipette");
  gtk_widget_set_visible(toggleButton, true);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggleButton), false);

  gtk_container_add(GTK_CONTAINER(button), toggleButton);
  g_signal_connect(static_cast<gpointer>(toggleButton), "toggled", G_CALLBACK(on_toggled), &handler->enabled);

  v->addToToolbar(button);

  gdk_threads_leave();

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// PipetteHandler
////////////////////////////////////////////////////////////////////////

PipetteHandler::PipetteHandler()
{
  selection = nullptr;
  enabled = false;
}
PipetteHandler::~PipetteHandler()
{
}

PipetteHandler::Ptr PipetteHandler::create()
{
  return Ptr(new PipetteHandler());
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void PipetteHandler::onSelectionStart(GdkPoint)
{
  if(enabled)
  {
    view->unsetPanning();
  }
}

void PipetteHandler::onSelectionUpdate(Selection* s)
{
  if(enabled)
  {
    selection = s;
  }
}

void PipetteHandler::onSelectionEnd(Selection* s)
{
  if(enabled)
  {
    selection = s;

    view->setPanning();

    // Get the image rectangle
    auto presentation = view->getCurrentPresentation();
    PresentationInterface::Ptr p = boost::static_pointer_cast<PresentationInterface>(presentation);
    auto image = p->getRect().toIntRectangle();

    // Get the selection rectangle
    unsigned int sel_x = std::min(selection->start.x, selection->end.x);
    unsigned int sel_y = std::min(selection->start.y, selection->end.y);
    auto sel_rect = Scroom::Utils::Rectangle<int>(sel_x, sel_y, selection->width(), selection->height());

    // Intersect both rectangles to get the part of the selection that overlaps the image
    auto rect = sel_rect.intersection(image);

    // Get the average color within the rectangle
    auto pipette = boost::static_pointer_cast<PipetteViewInterface>(presentation);
    auto colors = pipette->getAverages(rect);

    // Show the result on the status bar
    std::stringstream colours;
    colours << "Colors:";
    for (auto element : colors) {
      colours << ' ' << element.first.c_str() << ": " << element.second;
    }

    view->setStatusMessage(colours.str());
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void PipetteHandler::render(cairo_t* cr)
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