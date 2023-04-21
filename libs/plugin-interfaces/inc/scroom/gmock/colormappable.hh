/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gmock/gmock.h>

#include <scroom/colormappable.hh>

class ColormappableMock : public Colormappable
{
  MOCK_METHOD1(setColormap, void(Colormap::Ptr));
  MOCK_METHOD0(getOriginalColormap, Colormap::Ptr());
  MOCK_METHOD0(getNumberOfColors, int());
  MOCK_METHOD0(getMonochromeColor, Color());
  MOCK_METHOD1(setMonochromeColor, void(const Color&));
  MOCK_METHOD0(setTransparentBackground, void());
  MOCK_METHOD0(disableTransparentBackground, void());
  MOCK_METHOD0(getTransparentBackground, bool());
};
