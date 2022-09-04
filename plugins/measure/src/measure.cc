/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure.hh"

#include <fmt/format.h>

#include <gtk/gtk.h>

#include <scroom/cairo-helpers.hh>
#include <scroom/format_stuff.hh>
#include <scroom/impl/bookkeepingimpl.hh>
#include <scroom/point.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>

#include "version.h"

////////////////////////////////////////////////////////////////////////
// Measure
////////////////////////////////////////////////////////////////////////

Measure::Ptr Measure::create() { return Ptr(new Measure()); }

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Measure::getPluginName() { return "Measure"; }

std::string Measure::getPluginVersion() { return PACKAGE_VERSION; }

void Measure::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerViewObserver("Measure tool", shared_from_this<Measure>());
}

////////////////////////////////////////////////////////////////////////
// ViewObserver
////////////////////////////////////////////////////////////////////////

Scroom::Bookkeeping::Token Measure::viewAdded(ViewInterface::Ptr view)
{
  MeasureHandler::Ptr const handler = MeasureHandler::create();
  view->registerSelectionListener(handler);
  view->registerPostRenderer(handler);

  view->addToolButton(GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Measure")), handler);

  return {};
}

////////////////////////////////////////////////////////////////////////
// MeasureHandler
////////////////////////////////////////////////////////////////////////

MeasureHandler::Ptr MeasureHandler::create() { return Ptr(new MeasureHandler()); }

void MeasureHandler::displayMeasurement(const ViewInterface::Ptr& view)
{
  const auto      aspectRatio = view->getCurrentPresentation()->getAspectRatio();
  const Selection tweaked(selection->start / aspectRatio, selection->end / aspectRatio);

  view->setStatusMessage(fmt::format("l: {:.1f}, dx: {}, dy: {}, from: {}, to: {}",
                                     tweaked.length(),
                                     tweaked.width(),
                                     tweaked.height(),
                                     tweaked.start,
                                     tweaked.end));
}

void MeasureHandler::drawCross(cairo_t* cr, Scroom::Utils::Point<double> p)
{
  static const int size = 10;
  cairo_move_to(cr, p.x - size, p.y);
  cairo_line_to(cr, p.x + size, p.y);
  cairo_move_to(cr, p.x, p.y - size);
  cairo_line_to(cr, p.x, p.y + size);
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void MeasureHandler::onSelectionStart(Selection /*selection*/, ViewInterface::Ptr /*view*/) {}

void MeasureHandler::onSelectionUpdate(Selection s, ViewInterface::Ptr view)
{
  if(enabled)
  {
    selection = s;
    displayMeasurement(view);
  }
}

void MeasureHandler::onSelectionEnd(Selection s, ViewInterface::Ptr view)
{
  if(enabled)
  {
    selection = s;
    displayMeasurement(view);
  }
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void MeasureHandler::render(ViewInterface::Ptr const& /*vi*/,
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
    cairo_set_source_rgb(cr, 0.75, 0, 0); // Dark Red
    drawCross(cr, start);
    drawCross(cr, end);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1, 0, 0); // Red
    cairo_move_to(cr, start.x, start.y);
    cairo_line_to(cr, end.x, end.y);
    cairo_stroke(cr);
  }
}

////////////////////////////////////////////////////////////////////////
// ToolStateListener
////////////////////////////////////////////////////////////////////////

void MeasureHandler::onDisable()
{
  selection.reset();
  enabled = false;
}

void MeasureHandler::onEnable() { enabled = true; }
