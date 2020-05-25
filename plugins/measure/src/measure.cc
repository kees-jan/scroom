/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure.hh"

Measure::Measure()
{
}
Measure::~Measure()
{
}

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

Scroom::Bookkeeping::Token Measure::viewAdded(ViewInterface::Ptr view){
  printf("View added\n");

  MeasureHandler::Ptr handler = MeasureHandler::create();
  handler->view = view;
  view->registerSelectionListener(handler, MouseButton::SECONDARY);
  view->registerPostRenderer(handler);

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// MeasureHandler
////////////////////////////////////////////////////////////////////////

MeasureHandler::MeasureHandler()
{
}
MeasureHandler::~MeasureHandler()
{
}

MeasureHandler::Ptr MeasureHandler::create()
{
  return Ptr(new MeasureHandler());
}

void MeasureHandler::onSelection(Selection* selection)
{
  this->selection = selection;
  printf("Cookies received %d %d %d %d\n", selection->start.x, selection->end.x, selection->start.y, selection->end.y);
}

void MeasureHandler::render(cairo_t* cr)
{
  printf("Render called\n");
  if(this->selection){
	printf("Selection set\n");
	//GdkPoint start = presentationPointToWindowPoint(measurement->start);
	//GdkPoint end = presentationPointToWindowPoint(measurement->end);
	cairo_set_line_width(cr, 1);
	cairo_stroke(cr);
	cairo_set_source_rgb(cr, 0, 0, 1);
	cairo_move_to(cr, 0, 0);
	cairo_line_to(cr, 100, 100);
	//cairo_line_to(cr, start.x, end.y);
	//cairo_line_to(cr, end.x, end.y);
	cairo_stroke(cr);
  }
}
