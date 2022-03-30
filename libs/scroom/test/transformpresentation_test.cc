/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gmock/gmock-more-actions.h>
#include <gtest/gtest.h>

#include <scroom/bitmap-helpers.hh>
#include <scroom/dont-delete.hh>
#include <scroom/gmock/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/transformpresentation.hh>

using ::testing::_;
using ::testing::AtLeast;
using ::testing::ByRef;
using ::testing::DoAll;
using ::testing::Eq;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::Sequence;

using Scroom::Bitmap::BitmapSurface;
using Scroom::Utils::make_rect;

bool points_are_close(Scroom::Utils::Point<double> const& a, Scroom::Utils::Point<double> const& b)
{
  return (a - b).magnitude() < 1e-6;
}

bool rects_are_close(Scroom::Utils::Rectangle<double> const& a, Scroom::Utils::Rectangle<double> const& b)
{
  return points_are_close(a.getTopLeft(), b.getTopLeft()) && points_are_close(a.getBottomRight(), b.getBottomRight());
}

bool operator==(ViewInterface::WeakPtr const& a, ViewInterface::WeakPtr const& b) { return a.lock() == b.lock(); }

TEST(TransformPresentation_Tests, TransformationData_supports_aspect_ratio) // NOLINT
{
  TransformationData::Ptr td = TransformationData::create();
  td->setAspectRatio(2, 3);
  EXPECT_PRED2(points_are_close, Scroom::Utils::Point<double>(2, 3), td->getAspectRatio());

  ColormappablePresentationMock::Ptr cpm = ColormappablePresentationMock::create();

  TransformPresentation::Ptr tp = TransformPresentation::create(cpm, td);

  ViewInterface::Ptr const vi(reinterpret_cast<ViewInterface*>(1), DontDelete<ViewInterface>());
  EXPECT_CALL(*cpm, getRect()).WillRepeatedly(Return(make_rect(1.0, 2.0, 3.0, 4.0)));

  ViewInterface::WeakPtr viw;
  Sequence               seq;
  EXPECT_CALL(*cpm, open(_)).InSequence(seq).WillOnce(SaveArg<0>(&viw));
  const Scroom::Utils::Rectangle<double> to_be_drawn = make_rect(1.0, 4.5, 4.0, 9.0);
  const int                              zoom_to_use = 3;
  Scroom::Utils::Rectangle<double>       requested_to_be_drawn;
  int                                    used_zoom;
  EXPECT_CALL(*cpm, redraw(Eq(ByRef(viw)), _, _, _)).WillOnce(DoAll(SaveArg<2>(&requested_to_be_drawn), SaveArg<3>(&used_zoom)));
  EXPECT_CALL(*cpm, close(Eq(ByRef(viw))));

  Scroom::Utils::Rectangle<double> r = tp->getRect();
  EXPECT_PRED2(rects_are_close, make_rect(2.0, 6.0, 6.0, 12.0), r);

  tp->open(vi);

  BitmapSurface::Ptr s       = BitmapSurface::create(10, 10, CAIRO_FORMAT_ARGB32);
  cairo_surface_t*   surface = s->get();
  cairo_t*           cr      = cairo_create(surface);

  tp->redraw(vi, cr, to_be_drawn, zoom_to_use);

  EXPECT_PRED2(rects_are_close, make_rect(0.5, 1.5, 2.0, 3.0), requested_to_be_drawn);
  EXPECT_EQ(zoom_to_use, used_zoom);
  cairo_destroy(cr);

  tp->close(vi);
}
