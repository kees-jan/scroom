/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
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

gboolean reportComplete(gpointer data)
{
  if(data)
  {
    ((FileOperationObserver*)data)->fileOperationComplete();
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////

Example::~Example()
{
}

std::string Example::getPluginName()
{
  return "Example";
}

std::string Example::getPluginVersion()
{
  return "0.0";
}

void Example::registerCapabilities(ScroomInterface* host)
{
  host->registerNewInterface("Example", this);
}

void Example::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterNewInterface(this);
}

PresentationInterface::Ptr Example::createNew(FileOperationObserver* observer)
{
  if(observer)
  {
    gdk_threads_add_idle(reportComplete, observer);
  }
  return PresentationInterface::Ptr(new ExamplePresentation());
}
  