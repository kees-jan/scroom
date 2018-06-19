/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gmock/gmock.h>

#include <scroom/presentationinterface.hh>

#include <scroom/gmock/colormappable.hh>

class PresentationMock : public PresentationInterface
{
public:
  typedef boost::shared_ptr<PresentationMock> Ptr;
  
  static Ptr create() { return Ptr(new PresentationMock()); }
  
  MOCK_METHOD1(open, void (ViewInterface::WeakPtr));
  MOCK_METHOD1(close, void (ViewInterface::WeakPtr));

  MOCK_METHOD0(getRect, GdkRectangle());
  MOCK_METHOD4(redraw, void(ViewInterface::Ptr const&, cairo_t*, GdkRectangle, int));
  MOCK_METHOD2(getProperty, bool(const std::string&, std::string&));
  MOCK_METHOD1(isPropertyDefined, bool(const std::string&));
  MOCK_METHOD0(getTitle, std::string());
};

class ColormappablePresentationMock : public PresentationMock, public ColormappableMock
{
public:
  typedef boost::shared_ptr<ColormappablePresentationMock> Ptr;

  static Ptr create() { return Ptr(new ColormappablePresentationMock()); }
};
