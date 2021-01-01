#include "pipette.hh"

#include <cmath>

#include <gdk/gdk.h>

#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////
// Pipette
////////////////////////////////////////////////////////////////////////

Pipette::Ptr Pipette::create() { return Ptr(new Pipette()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Pipette::getPluginName() { return "Pipette"; }

std::string Pipette::getPluginVersion() { return "0.0"; }

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

PipetteHandler::Ptr PipetteHandler::create() { return Ptr(new PipetteHandler()); }

void PipetteHandler::computeValues(ViewInterface::Ptr view, Scroom::Utils::Rectangle<int> sel_rect)
{
  jobMutex.lock();

  gdk_threads_enter();
  view->setStatusMessage("Computing color values...");
  gdk_threads_leave();

  // Get the average color within the rectangle
  PresentationInterface::Ptr presentation = view->getCurrentPresentation();
  auto                       pipette      = boost::dynamic_pointer_cast<PipetteViewInterface>(presentation);
  if(pipette == nullptr || !presentation->isPropertyDefined(PIPETTE_PROPERTY_NAME))
  {
    printf("PANIC: Presentation does not implement PipetteViewInterface!\n");
    gdk_threads_enter();
    view->setStatusMessage("Pipette is not supported for this presentation.");
    gdk_threads_leave();
    jobMutex.unlock();
    return;
  }
  auto image  = presentation->getRect().toIntRectangle();
  auto rect   = sel_rect.intersection(image);
  auto colors = pipette->getPixelAverages(rect);

  // If the plugin was switched off ignore the result
  if(!wasDisabled.test_and_set())
  {
    displayValues(view, rect, colors);
  }

  wasDisabled.clear();
  jobMutex.unlock();
}

void PipetteHandler::displayValues(ViewInterface::Ptr                   view,
                                   Scroom::Utils::Rectangle<int>        rect,
                                   PipetteLayerOperations::PipetteColor colors)
{
  std::stringstream info;
  info.precision(2);

  info << "Top-left: " << rect.getTopLeft();
  info << ", Bottom-right: " << rect.getBottomRight();
  info << ", Height: " << rect.getHeight();
  info << ", Width: " << rect.getWidth();
  if(!colors.empty())
  {
    info << ", Colors:";
    for(const auto& element: colors)
    {
      info << ' ' << element.first << ": " << std::fixed << element.second;
    }
  }

  gdk_threads_enter();
  view->setStatusMessage(info.str());
  gdk_threads_leave();
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void PipetteHandler::onSelectionStart(GdkPoint, ViewInterface::Ptr) {}

void PipetteHandler::onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view)
{
  UNUSED(view);
  if(enabled && jobMutex.try_lock())
  {
    selection = s;
    jobMutex.unlock();
  }
}

void PipetteHandler::onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view)
{
  if(enabled && jobMutex.try_lock())
  {
    selection = s;

    // Get the selection rectangle
    auto sel_rect = Scroom::Utils::Rectangle<int>(selection->start, selection->end);
    Sequentially()->schedule(boost::bind(&PipetteHandler::computeValues, shared_from_this<PipetteHandler>(), view, sel_rect),
                             currentJob);
    jobMutex.unlock();
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void PipetteHandler::render(ViewInterface::Ptr const&        vi,
                            cairo_t*                         cr,
                            Scroom::Utils::Rectangle<double> presentationArea,
                            int                              zoom)
{
  if(selection)
  {
    auto aspectRatio = vi->getCurrentPresentation()->getAspectRatio();
    auto start       = Scroom::Utils::Point<int>(selection->start) - presentationArea.getTopLeft();
    auto end         = Scroom::Utils::Point<int>(selection->end) - presentationArea.getTopLeft();

    if(zoom >= 0)
    {
      const int pixelSize = 1 << zoom;
      start *= pixelSize;
      start *= aspectRatio;
      end *= pixelSize;
      end *= aspectRatio;
    }
    else
    {
      const int pixelSize = 1 << -zoom;
      start *= aspectRatio;
      start /= pixelSize;
      end *= aspectRatio;
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

void PipetteHandler::onDisable()
{
  selection = nullptr;
  enabled   = false;
  wasDisabled.test_and_set();
}

void PipetteHandler::onEnable()
{
  enabled = true;
  if(jobMutex.try_lock())
  {
    wasDisabled.clear();
    jobMutex.unlock();
  }
}
