/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
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

#include "transparentoverlay.hh"

#include <gdk/gdk.h>

#include "transparentoverlaypresentation.hh"

////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////

TransparentOverlay::TransparentOverlay()
{
}
TransparentOverlay::~TransparentOverlay()
{
}

TransparentOverlay::Ptr TransparentOverlay::create()
{
  return Ptr(new TransparentOverlay());
}

std::string TransparentOverlay::getPluginName()
{
  return "TransparentOverlay";
}

std::string TransparentOverlay::getPluginVersion()
{
  return "0.0";
}

void TransparentOverlay::registerCapabilities(ScroomInterface::Ptr host)
{
  host->registerNewInterface("TransparentOverlay", shared_from_this<TransparentOverlay>());
}

PresentationInterface::Ptr TransparentOverlay::createNew()
{
  return PresentationInterface::Ptr(new TransparentOverlayPresentation());
}
  
