/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "pipette.hh"

#include <spdlog/spdlog.h>

#include <gdk/gdk.h>

#include <scroom/cairo-helpers.hh>
#include <scroom/format_stuff.hh>
#include <scroom/unused.hh>

#include "version.h"

////////////////////////////////////////////////////////////////////////
// Pipette
////////////////////////////////////////////////////////////////////////

Pipette::Ptr Pipette::create() { return Ptr(new Pipette()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Pipette::getPluginName() { return "Pipette"; }

std::string Pipette::getPluginVersion() { return PACKAGE_VERSION; }

void Pipette::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerViewObserver("Pipette", shared_from_this<Pipette>());
}

////////////////////////////////////////////////////////////////////////
// ViewObserver
////////////////////////////////////////////////////////////////////////

Scroom::Bookkeeping::Token Pipette::viewAdded(ViewInterface::Ptr view)
{
  PipetteHandler::Ptr const handler = PipetteHandler::create();
  view->registerSelectionListener(handler);
  view->registerPostRenderer(handler);

  view->addToolButton(GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Pipette")), handler);

  return {};
}

////////////////////////////////////////////////////////////////////////
// PipetteHandler
////////////////////////////////////////////////////////////////////////

PipetteHandler::Ptr PipetteHandler::create() { return Ptr(new PipetteHandler()); }

void PipetteHandler::computeValues(const ViewInterface::Ptr& view, Scroom::Utils::Rectangle<double> sel_rect)
{
  jobMutex.lock();

  Scroom::GtkHelpers::sync_on_ui_thread([=] { view->setStatusMessage("Computing color values..."); });

  // Get the average color within the rectangle
  PresentationInterface::Ptr const presentation = view->getCurrentPresentation();
  auto                             pipette      = boost::dynamic_pointer_cast<PipetteViewInterface>(presentation);
  if(pipette == nullptr || !presentation->isPropertyDefined(PIPETTE_PROPERTY_NAME))
  {
    spdlog::error("Presentation does not implement PipetteViewInterface!");
    Scroom::GtkHelpers::sync_on_ui_thread([=] { view->setStatusMessage("Pipette is not supported for this presentation."); });
    jobMutex.unlock();
    return;
  }
  auto image  = presentation->getRect();
  auto rect   = sel_rect.intersection(image);
  auto colors = pipette->getPixelAverages(rect);

  const auto display_rect = roundOutward(rect / presentation->getAspectRatio());

  // If the plugin was switched off ignore the result
  if(!wasDisabled.test_and_set())
  {
    displayValues(view, display_rect, colors);
  }

  wasDisabled.clear();
  jobMutex.unlock();
}

void PipetteHandler::displayValues(const ViewInterface::Ptr&                   view,
                                   Scroom::Utils::Rectangle<double>            rect,
                                   const PipetteLayerOperations::PipetteColor& colors)
{
  std::stringstream info;

  info << fmt::format("Top-left: {}, Bottom-right: {}, Height: {}, Width: {}",
                      rect.getTopLeft(),
                      rect.getBottomRight(),
                      rect.getHeight(),
                      rect.getWidth());

  if(!colors.empty())
  {
    info << ", Colors:";
    for(const auto& [name, value]: colors)
    {
      info << fmt::format(" {}: {:.2f}", name, value);
    }
  }

  Scroom::GtkHelpers::sync_on_ui_thread([view, status = info.str()] { view->setStatusMessage(status); });
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

std::string PipetteHandler::getSelectionType() { return SelectionType::PIXEL; }

void PipetteHandler::onSelectionStart(Selection /*selection*/, ViewInterface::Ptr /*view*/) {}

void PipetteHandler::onSelectionUpdate(Selection s, ViewInterface::Ptr view)
{
  UNUSED(view);
  if(enabled && jobMutex.try_lock())
  {
    selection = s;
    jobMutex.unlock();
  }
}

void PipetteHandler::onSelectionEnd(Selection s, ViewInterface::Ptr view)
{
  if(enabled && jobMutex.try_lock())
  {
    selection = s;

    // Get the selection rectangle
    const auto sel_rect = Scroom::Utils::make_rect_from_start_end(selection->start, selection->end);
    Sequentially()->schedule([me = shared_from_this<PipetteHandler>(), view, sel_rect] { me->computeValues(view, sel_rect); },
                             currentJob);
    jobMutex.unlock();
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void PipetteHandler::render(ViewInterface::Ptr const& /*vi*/,
                            cairo_t*                         cr,
                            Scroom::Utils::Rectangle<double> presentationArea,
                            int                              zoom)
{
  if(selection)
  {
    const auto pixelSize = pixelSizeFromZoom(zoom);
    const auto start     = (selection->start - presentationArea.getTopLeft()) * pixelSize;
    const auto end       = (selection->end - presentationArea.getTopLeft()) * pixelSize;

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
  selection.reset();
  enabled = false;
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
