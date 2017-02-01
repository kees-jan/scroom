/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/colormappable.hh>
#include <scroom/unused.hh>

////////////////////////////////////////////////////////////////////////
// ColormapHelperBase

ColormapHelperBase::ColormapHelperBase(Colormap::Ptr const& colormap)
  : colormap(colormap), originalColormap(colormap)
{
}

void ColormapHelperBase::setColormap(Colormap::Ptr colormap)
{
  this->colormap = colormap;
}

void ColormapHelperBase::setOriginalColormap(Colormap::Ptr colormap)
{
  this->originalColormap = colormap;
}

Colormap::Ptr ColormapHelperBase::getOriginalColormap()
{
  return originalColormap;
}

int ColormapHelperBase::getNumberOfColors()
{
  return originalColormap->colors.size();
}

Color ColormapHelperBase::getMonochromeColor()
{
  OperationNotSupported();
}

void ColormapHelperBase::setMonochromeColor(const Color& c)
{
  UNUSED(c);
  OperationNotSupported();
}

void ColormapHelperBase::setTransparentBackground()
{
  OperationNotSupported();
}

void ColormapHelperBase::disableTransparentBackground()
{
  OperationNotSupported();
}

bool ColormapHelperBase::getTransparentBackground()
{
  OperationNotSupported();
}

Colormap::Ptr ColormapHelperBase::getColormap()
{
  return colormap;
}

void ColormapHelperBase::OperationNotSupported()
{
  throw std::runtime_error("Operation not supported");
}

////////////////////////////////////////////////////////////////////////
// ColormapHelper

ColormapHelper::Ptr ColormapHelper::create(Colormap::Ptr const& colormap)
{
  return Ptr(new ColormapHelper(colormap));
}

ColormapHelper::Ptr ColormapHelper::create(int numberOfColors)
{
  return create(Colormap::createDefault(numberOfColors));
}

ColormapHelper::Ptr ColormapHelper::createInverted(int numberOfColors)
{
  return create(Colormap::createDefaultInverted(numberOfColors));
}

ColormapHelper::ColormapHelper(Colormap::Ptr const& colormap)
  : ColormapHelperBase(colormap)
{
}

