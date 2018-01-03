/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "example.hh"

#include <gdk/gdk.h>

#include "examplepresentation.hh"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

Example::Example()
{
}
Example::~Example()
{
}

Example::Ptr Example::create()
{
  return Ptr(new Example());
}

std::string Example::getPluginName()
{
  return "Example";
}

std::string Example::getPluginVersion()
{
  return "0.0";
}

void Example::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerNewPresentationInterface("Example", shared_from_this<Example>());
}

PresentationInterface::Ptr Example::createNew()
{
  return PresentationInterface::Ptr(new ExamplePresentation());
}
  
