/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
  
