/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure.hh"

////////////////////////////////////////////////////////////////////////
// Measure
////////////////////////////////////////////////////////////////////////

Measure::Ptr Measure::create()
{
  return Ptr(new Measure());
}

////////////////////////////////////////////////////////////////////////
// PluginInformationInterface
////////////////////////////////////////////////////////////////////////

std::string Measure::getPluginName()
{
  return "Measure";
}

std::string Measure::getPluginVersion()
{
  return "0.0";
}

void Measure::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerViewObserver("Measure tool", shared_from_this<Measure>());
}

////////////////////////////////////////////////////////////////////////
// ViewObserver
////////////////////////////////////////////////////////////////////////

Scroom::Bookkeeping::Token Measure::viewAdded(ViewInterface::Ptr view)
{
  MeasureHandler::Ptr handler = MeasureHandler::create();
  view->registerSelectionListener(handler);
  view->registerPostRenderer(handler);

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// MeasureHandler
////////////////////////////////////////////////////////////////////////

MeasureHandler::MeasureHandler()
{
  selection = nullptr;
}
MeasureHandler::~MeasureHandler()
{
}

MeasureHandler::Ptr MeasureHandler::create()
{
  return Ptr(new MeasureHandler());
}

void MeasureHandler::displayMeasurement(ViewInterface::Ptr view)
{
  std::ostringstream s;
  s.precision(1);
  fixed(s);

  s << "l: " << selection->length()
    << ", dx: " << selection->width()
    << ", dy: " << selection->height()
    << ", from: ("<< selection->start.x << "," << selection->start.y << ")"
    << ", to: ("<< selection->end.x << "," << selection->end.y << ")";

  view->setStatusMessage(s.str());
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void MeasureHandler::onSelectionStart(GdkPoint, ViewInterface::Ptr)
{
}

void MeasureHandler::onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view)
{
  selection = s;
  displayMeasurement(view);
}

void MeasureHandler::onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view)
{
  selection = s;
  displayMeasurement(view);
}

////////////////////////////////////////////////////////////////////////
// PostRenderer
////////////////////////////////////////////////////////////////////////

void MeasureHandler::render(cairo_t* cr, ViewInterface::Ptr view)
{
  if(selection)
  {
    GdkPoint start = view->presentationPointToWindowPoint(selection->start);
    GdkPoint end = view->presentationPointToWindowPoint(selection->end);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1, 0, 0); // Red
    cairo_move_to(cr, start.x, start.y);
    cairo_line_to(cr, end.x, end.y);
    cairo_line_to(cr, end.x, start.y);
    cairo_line_to(cr, start.x, start.y);
    cairo_line_to(cr, start.x, end.y);
    cairo_line_to(cr, end.x, end.y);
    cairo_stroke(cr);
  }
}
