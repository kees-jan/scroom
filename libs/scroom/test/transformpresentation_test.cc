/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <gmock/gmock-more-actions.h>

#include <scroom/dont-delete.hh>
#include <scroom/gmock/presentationinterface.hh>
#include <scroom/transformpresentation.hh>
#include <scroom/rectangle.hh>

using ::testing::AtLeast;
using ::testing::Return;
using ::testing::_;
using ::testing::SaveArg;
using ::testing::DoAll;

bool points_are_close(Point<double> const& a, Point<double> const& b)
{
  return (a-b).magnitude() < 1e-6;
}

bool rects_are_close(Rectangle<double> const& a, Rectangle<double> const& b)
{
  return
    points_are_close(a.getTopLeft(), b.getTopLeft()) &&
    points_are_close(a.getBottomRight(), b.getBottomRight());
}

bool operator==(ViewInterface::WeakPtr const& a, ViewInterface::WeakPtr const& b)
{
  return a.lock() == b.lock();
}

TEST(TransformPresentation_Tests, TransformationData_supports_aspect_ratio)
{
  TransformationData::Ptr td = TransformationData::create();
  td->setAspectRatio(2,3);
  EXPECT_PRED2(points_are_close, Point<double>(2,3), td->getAspectRatio());

  ColormappablePresentationMock::Ptr cpm = ColormappablePresentationMock::create();
  
  TransformPresentation::Ptr tp = TransformPresentation::create(cpm, td);

  ViewInterface::Ptr const vi(reinterpret_cast<ViewInterface*>(1), DontDelete<ViewInterface>());
  ViewInterface::WeakPtr const viw(vi);
  
  EXPECT_CALL(*cpm, getRect()).WillRepeatedly(Return(make_rect(1.0, 2.0, 3.0, 4.0)));
  EXPECT_CALL(*cpm, open(viw));
  EXPECT_CALL(*cpm, close(viw));
  const Rectangle<double> to_be_drawn = make_rect(0.5, 1.5, 2.0, 3.0);
  const int zoom_to_use = 3;
  Rectangle<double> requested_to_be_drawn;
  int used_zoom;
  EXPECT_CALL(*cpm, redraw(vi, NULL, to_be_drawn, zoom_to_use))
    .WillOnce(DoAll(SaveArg<2>(&requested_to_be_drawn),
                    SaveArg<3>(&used_zoom)));

  Rectangle<double> r = tp->getRect();
  EXPECT_PRED2(rects_are_close, make_rect(2.0, 6.0, 6.0, 12.0), r);

  tp->open(vi);

  tp->redraw(vi, NULL, to_be_drawn, zoom_to_use);
  // EXPECT_PRED2(rects_are_close, make_rect(1.0, 4.5, 4.0, 9.0), requested_to_be_drawn);
  EXPECT_EQ(zoom_to_use, used_zoom);
  
  tp->close(vi);
}
