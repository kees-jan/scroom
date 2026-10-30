/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <scroom/gmock/tweak-view.hh>
#include <scroom/tweak-view.hh>

#include "tweak-view-test-helpers.hh"

using namespace Scroom::Utils;

namespace
{
  constexpr int ZOOMED_IN_BY_FACTOR_2 = 1;
  constexpr int ZOOMED_OUT_BY_FACTOR_2 = -1;

  Selection scaledBy(const Selection& selection, Point<double> scale) { return {selection.start * scale, selection.end * scale}; }
} // namespace

namespace
{
  class TweakPresentationPositionsWithAspectRatioTest : public testing::Test
  {
  protected:
    const Point<double> m_aspectRatio{2, 3};
    const std::shared_ptr<testing::StrictMock<PresentationPositionTweakMock>> m_child =
      std::make_shared<testing::StrictMock<PresentationPositionTweakMock>>();
    const ITweakPresentationPosition::Ptr m_tweak = TweakPresentationPositionsWithAspectRatio::create(m_child, m_aspectRatio);
  };
} // namespace

TEST_F(TweakPresentationPositionsWithAspectRatioTest, ConvertsTheInputToChildCoordinatesAndScalesTheChildOutput)
{
  const Point<double> input{1.25, -2.75};
  const Point<double> childOutput{-3.25, 4.75};
  EXPECT_CALL(*m_child, tweakPosition(pointEquals(input / m_aspectRatio), ZOOMED_IN_BY_FACTOR_2))
    .WillOnce(testing::Return(childOutput));

  EXPECT_THAT(m_tweak->tweakPosition(input, ZOOMED_IN_BY_FACTOR_2), pointEquals(childOutput * m_aspectRatio));
}

namespace
{
  class TweakRulersWithAspectRatioTest : public testing::Test
  {
  protected:
    const Point<double> m_aspectRatio{2, 3};
    const std::shared_ptr<testing::StrictMock<RulerAreaTweakMock>> m_child =
      std::make_shared<testing::StrictMock<RulerAreaTweakMock>>();
    const ITweakRulers::Ptr m_tweak = TweakRulersWithAspectRatio::create(m_child, m_aspectRatio);
  };
} // namespace

TEST_F(TweakRulersWithAspectRatioTest, PassesThePositionToTheChildAndScalesTheReturnedArea)
{
  const Point<double> viewPosition{1.24, -1.49};
  const Point<int> viewSize{101, 83};
  const Point<double> childViewPosition{.25, -.75};
  const Point<double> childAreaSize{25.25, 41.5};
  EXPECT_CALL(*m_child, tweakRulers(pointEquals(viewPosition), viewSize, ZOOMED_IN_BY_FACTOR_2))
    .WillOnce(testing::Return(make_rect(childViewPosition, childAreaSize)));

  const auto result = m_tweak->tweakRulers(viewPosition, viewSize, ZOOMED_IN_BY_FACTOR_2);

  EXPECT_THAT(result.getTopLeft(), pointEquals(childViewPosition / m_aspectRatio));
  EXPECT_THAT(result.getSize(), pointEquals(childAreaSize / m_aspectRatio));
}

namespace
{
  class TweakSelectionsWithAspectRatioTest : public testing::Test
  {
  protected:
    const Point<double> m_aspectRatio{2, 3};
    const std::shared_ptr<testing::StrictMock<SelectionTweakMock>> m_child =
      std::make_shared<testing::StrictMock<SelectionTweakMock>>();
    const ITweakSelection::Ptr m_tweak = TweakSelectionsWithAspectRatio::create(m_child, m_aspectRatio);
  };
} // namespace

TEST_F(TweakSelectionsWithAspectRatioTest, ConvertsBothInputEndpointsAndScalesBothChildEndpoints)
{
  const Selection input{{1.25, -2.75}, {-3.25, 4.75}};
  const Selection childOutput{{7, -8}, {-9, 10}};
  const Point<double> inverseAspectRatio = Point<double>{1, 1} / m_aspectRatio;
  EXPECT_CALL(*m_child, tweakSelection(selectionEquals(scaledBy(input, inverseAspectRatio))))
    .WillOnce(testing::Return(childOutput));

  EXPECT_THAT(m_tweak->tweakSelection(input), selectionEquals(scaledBy(childOutput, m_aspectRatio)));
}

namespace
{
  class ParseAndFormatViewCentreWithAspectRatioTest : public testing::Test
  {
  protected:
    const Point<double> m_aspectRatio{2, 3};
    const Point<int> m_viewSize{101, 83};
    const std::shared_ptr<testing::StrictMock<ViewCentreTextConversionMock>> m_child =
      std::make_shared<testing::StrictMock<ViewCentreTextConversionMock>>();
    const ITweakPositionTextBox::Ptr m_parseAndFormat = ParseAndFormatViewCentreWithAspectRatio::create(m_child, m_aspectRatio);
  };
} // namespace

TEST_F(ParseAndFormatViewCentreWithAspectRatioTest, ScalesTheParsedCentreAndReturnsTheViewOrigin)
{
  const Point<double> childViewPosition{1.25, -2.75};
  constexpr double magnification = 2;
  const auto viewSizeInPresentationCoordinates = m_viewSize.to<double>() / magnification;
  const auto halfViewSizeInPresentationCoordinates = viewSizeInPresentationCoordinates / 2;
  const auto childCentre = childViewPosition + halfViewSizeInPresentationCoordinates;
  const auto expectedOrigin = childCentre * m_aspectRatio - halfViewSizeInPresentationCoordinates;
  EXPECT_CALL(*m_child, parse("some x", "some y", m_viewSize, ZOOMED_IN_BY_FACTOR_2))
    .WillOnce(testing::Return(childViewPosition));

  EXPECT_THAT(m_parseAndFormat->parse("some x", "some y", m_viewSize, ZOOMED_IN_BY_FACTOR_2), pointEquals(expectedOrigin));
}

TEST_F(ParseAndFormatViewCentreWithAspectRatioTest, ConvertsTheCentreBeforeAskingTheChildToDisplayIt)
{
  const Point<double> viewPosition{1.25, -2.75};
  const auto viewSizeInPresentationCoordinates = m_viewSize.to<double>() * 2;
  const auto halfViewSizeInPresentationCoordinates = viewSizeInPresentationCoordinates / 2;
  const auto viewCentre = viewPosition + halfViewSizeInPresentationCoordinates;
  const auto childViewPosition = viewCentre / m_aspectRatio - halfViewSizeInPresentationCoordinates;
  const std::pair<std::string, std::string> childText{"child x", "child y"};
  EXPECT_CALL(*m_child, display(pointEquals(childViewPosition), m_viewSize, ZOOMED_OUT_BY_FACTOR_2))
    .WillOnce(testing::Return(childText));

  EXPECT_EQ(childText, m_parseAndFormat->display(viewPosition, m_viewSize, ZOOMED_OUT_BY_FACTOR_2));
}
