/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2011 Kees-Jan Dijkzeul
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

#include "colormapplugin.hh"

#include <scroom/colormappable.hh>

#include "colormapprovider.hh"

////////////////////////////////////////////////////////////////////////

ColormapPlugin::ColormapPlugin()
  : presentations()
{
}

ColormapPlugin::~ColormapPlugin()
{
}

std::string ColormapPlugin::getPluginName()
{
  return "Colormap";
}

std::string ColormapPlugin::getPluginVersion()
{
  return "0.0";
}

void ColormapPlugin::registerCapabilities(ScroomInterface* host)
{
  host->registerPresentationObserver("Colormap", this);
}

void ColormapPlugin::unregisterCapabilities(ScroomInterface* host)
{
  host->unregisterPresentationObserver(this);
}

void ColormapPlugin::presentationAdded(PresentationInterface::Ptr p)
{
  printf("ColormapPlugin: A presentation was created\n");
  if(p->isPropertyDefined(COLORMAPPABLE_PROPERTY_NAME))
  {
    printf("ColormapPlugin: It is colormappable!\n");
    ColormapProvider::Ptr cmp = ColormapProvider::create(p);
  }
}

void ColormapPlugin::presentationDeleted()
{
  printf("ColormapPlugin: A presentation may have been deleted\n");
}
  
