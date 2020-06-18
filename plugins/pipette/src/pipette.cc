#include "pipette.hh"

#include <gdk/gdk.h>
#include <cmath>

#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////
// Pipette
////////////////////////////////////////////////////////////////////////

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

Scroom::Bookkeeping::Token Pipette::viewAdded(ViewInterface::Ptr view)
{
  PipetteHandler::Ptr handler = PipetteHandler::create();
  view->registerSelectionListener(handler);
  view->registerPostRenderer(handler);

  view->addToolButton(GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Pipette")), handler);

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// PipetteHandler
////////////////////////////////////////////////////////////////////////

PipetteHandler::PipetteHandler()
  : selection(nullptr), enabled(false), pendingJobs(ThreadPool::Queue::createAsync())
{
}
PipetteHandler::~PipetteHandler()
{
}

PipetteHandler::Ptr PipetteHandler::create()
{
  return Ptr(new PipetteHandler());
}

void PipetteHandler::computeValues(ViewInterface::Ptr view)
{
  gdk_threads_enter();
  view->setStatusMessage("Computing color values...");
  gdk_threads_leave();

  // Get the image rectangle
  PresentationInterface::Ptr presentation = view->getCurrentPresentation();
  if(presentation == nullptr)
  {
    // No current presentation in the view
    enabled = true;
    return;
  }
  auto image = presentation->getRect().toIntRectangle();

  // Get the selection rectangle
  int sel_x = std::min(selection->start.x, selection->end.x);
  int sel_y = std::min(selection->start.y, selection->end.y);
  auto sel_rect = Scroom::Utils::Rectangle<int>(sel_x, sel_y, selection->width(), selection->height());

  // Intersect both rectangles to get the part of the selection that overlaps the image
  auto rect = sel_rect.intersection(image);

  // Get the average color within the rectangle
  auto pipette = boost::dynamic_pointer_cast<PipetteViewInterface>(presentation);
  if(pipette == nullptr)
  {
    printf("PANIC: Presentation does not implement PipetteViewInterface!\n");
    gdk_threads_enter();
    view->setStatusMessage("Error when requesting the image data.");
    gdk_threads_leave();
    enabled = true;
    return;
  }
  auto colors = pipette->getPixelAverages(rect);

  // If selection became null the plugin was switched off so ignore the result
  if(selection == nullptr)
  {
    return;
  }
  enabled = true;

  displayValues(view, rect, colors);
}

void PipetteHandler::displayValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> rect, PipetteLayerOperations::PipetteColor colors)
{
  std::stringstream info;
  info.precision(2);
  fixed(info);

  info << "Top-left: " << rect.getTopLeft();
  info << ", Bottom-right: " << rect.getBottomRight();
  info << ", Height: " << rect.getHeight();
  info << ", Width: " << rect.getWidth();
  if(!colors.empty()){
    info << ", Colors:";
    for(auto element : colors)
    {
      info << ' ' << element.first << ": " << element.second;
    }
  }

  gdk_threads_enter();
  view->setStatusMessage(info.str());
  gdk_threads_leave();
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void PipetteHandler::onSelectionStart(GdkPoint, ViewInterface::Ptr)
{
}

void PipetteHandler::onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view)
{
  UNUSED(view);
  if(enabled)
  {
    selection = s;
  }
}

void PipetteHandler::onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view)
{
  if(enabled)
  {
    selection = s;

    // Prevent more than one job
    enabled = false;
    Sequentially()->schedule(boost::bind(&PipetteHandler::computeValues, shared_from_this<PipetteHandler>(), view), pendingJobs);
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void PipetteHandler::render(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom)
{
  UNUSED(vi);

  if(selection)
  {
    auto start = Scroom::Utils::Point<int>(selection->start) - presentationArea.getTopLeft();
    auto end = Scroom::Utils::Point<int>(selection->end) - presentationArea.getTopLeft();

    if(zoom>=0)
    {
      const int pixelSize=1<<zoom;
      start *= pixelSize;
      end *= pixelSize;
    }
    else
    {
      const int pixelSize=1<<-zoom;
      start /= pixelSize;
      end /= pixelSize;
    }

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

////////////////////////////////////////////////////////////////////////
// ToolStateListener
////////////////////////////////////////////////////////////////////////

void PipetteHandler::onDisable(){
  selection = nullptr;
  enabled = false;
}

void PipetteHandler::onEnable(){
  enabled = true;
}
