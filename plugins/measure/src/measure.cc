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

  view->registerSelectionListener(MeasureListener::create(), MouseButton::SECONDARY);
  view->registerPostRenderer(MeasureRenderer::create());

  return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// MeasureListener
////////////////////////////////////////////////////////////////////////

MeasureListener::MeasureListener(){
}

MeasureListener::~MeasureListener(){
}

MeasureListener::Ptr MeasureListener::create(){
  return Ptr(new MeasureListener());
}

void MeasureListener::onSelection(Selection* selection){
  printf("Cookies received %d %d %d %d\n", selection->start.x, selection->end.x, selection->start.y, selection->end.y);
}

////////////////////////////////////////////////////////////////////////
// MeasureRenderer
////////////////////////////////////////////////////////////////////////

MeasureRenderer::MeasureRenderer(){
}

MeasureRenderer::~MeasureRenderer(){
}

MeasureRenderer::Ptr MeasureRenderer::create(){
	return Ptr(new MeasureRenderer());
}

void MeasureRenderer::render(cairo_t* cr){
	printf("Render called");
}
