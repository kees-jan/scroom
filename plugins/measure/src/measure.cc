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

	view->registerSelectionListener(Listener::create(), MouseButton::SECONDARY);

	return Scroom::Bookkeeping::Token();
}

////////////////////////////////////////////////////////////////////////
// Listener
////////////////////////////////////////////////////////////////////////

Listener::Listener(){
}

Listener::~Listener(){
}

Listener::Ptr Listener::create(){
	return Ptr(new Listener());
}

void Listener::onSelection(Selection* selection){
	printf("Cookies received %d %d %d %d\n", selection->start.x, selection->end.x, selection->start.y, selection->end.y);
}
