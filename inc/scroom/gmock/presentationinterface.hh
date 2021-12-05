/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gmock/gmock.h>

#include <scroom/gmock/colormappable.hh>
#include <scroom/presentationinterface.hh>

class PresentationMock : public PresentationInterface
{
public:
  using Ptr = boost::shared_ptr<PresentationMock>;

  static Ptr create() { return Ptr(new PresentationMock()); }

  MOCK_METHOD1(open, void(ViewInterface::WeakPtr));
  MOCK_METHOD1(close, void(ViewInterface::WeakPtr));

  MOCK_METHOD0(getRect, Scroom::Utils::Rectangle<double>());
  MOCK_METHOD4(redraw, void(ViewInterface::Ptr const&, cairo_t*, Scroom::Utils::Rectangle<double>, int));
  MOCK_METHOD2(getProperty, bool(const std::string&, std::string&));
  MOCK_METHOD1(isPropertyDefined, bool(const std::string&));
  MOCK_METHOD0(getTitle, std::string());
  MOCK_METHOD0(showMetadata, void());
};

class ColormappablePresentationMock
  : public PresentationMock
  , public ColormappableMock
{
public:
  using Ptr = boost::shared_ptr<ColormappablePresentationMock>;

  static Ptr create() { return Ptr(new ColormappablePresentationMock()); }
};
