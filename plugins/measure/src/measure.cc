/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "measure.hh"
#include <scroom/unused.hh>

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

  view->addToolButton(GTK_TOGGLE_BUTTON(gtk_toggle_button_new_with_label("Measure")), handler);

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// MeasureHandler
////////////////////////////////////////////////////////////////////////

MeasureHandler::MeasureHandler()
  : selection(nullptr), enabled(false)
{
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

void MeasureHandler::drawCross(cairo_t* cr, Scroom::Utils::Point<double> p)
{
  static const int size = 10;
  cairo_move_to(cr, p.x-size, p.y);
  cairo_line_to(cr, p.x+size, p.y);
  cairo_move_to(cr, p.x, p.y-size);
  cairo_line_to(cr, p.x, p.y+size);
}

////////////////////////////////////////////////////////////////////////
// SelectionListener
////////////////////////////////////////////////////////////////////////

void MeasureHandler::onSelectionStart(GdkPoint, ViewInterface::Ptr)
{
}

void MeasureHandler::onSelectionUpdate(Selection::Ptr s, ViewInterface::Ptr view)
{
  if(enabled)
  {
    selection = s;
    displayMeasurement(view);
  }
}

void MeasureHandler::onSelectionEnd(Selection::Ptr s, ViewInterface::Ptr view)
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

void MeasureHandler::render(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom)
{
  UNUSED(vi);

  if(selection)
  {
    auto aspectRatio = vi->getCurrentPresentation()->getAspectRatio();
    auto start = Scroom::Utils::Point<int>(selection->start) - presentationArea.getTopLeft();
    auto end = Scroom::Utils::Point<int>(selection->end) - presentationArea.getTopLeft();

    if(zoom>=0)
    {
      const int pixelSize=1<<zoom;
      start *= pixelSize;
      start *= aspectRatio;
      end *= pixelSize;
      end *= aspectRatio;
    }
    else
    {
      const int pixelSize=1<<-zoom;
      start /= pixelSize;
      start /= aspectRatio;
      end /= pixelSize;
      end /= aspectRatio;
    }

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

void MeasureHandler::onDisable(){
  selection = nullptr;
  enabled = false;
}

void MeasureHandler::onEnable(){
  enabled = true;
}
