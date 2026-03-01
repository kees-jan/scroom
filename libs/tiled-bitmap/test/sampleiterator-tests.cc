/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstring>
#include <tuple>

#include <gtest/gtest.h>

#include <scroom/bitmap-helpers.hh>

using Scroom::Bitmap::SampleIterator;

namespace Scroom::Bitmap
{
  template <typename T>
  std::ostream& operator<<(std::ostream& os, const SampleIterator<T>& it)
  {
    return os << '(' << static_cast<const void*>(it.currentBase) << ", " << it.currentOffset << ", " << it.bps << ')';
  }
} // namespace Scroom::Bitmap

namespace
{
  const uint8_t testData[]        = {0x65, 0xC7};
  const int     bit_depths[]      = {1, 2, 4, 8};
  const int     initial_offsets[] = {0, 1};
  const int     deltas[]          = {0, 1, 5};
} // namespace

class SampleIterator_iterates : public ::testing::TestWithParam<int>
{
};

INSTANTIATE_TEST_SUITE_P(SampleIterator_Tests, SampleIterator_iterates, ::testing::ValuesIn(bit_depths));

TEST_P(SampleIterator_iterates, Test) // NOLINT
{
  const int                     bps = GetParam();
  SampleIterator<const uint8_t> it(testData, 0, bps);
  uint8_t                       output[] = {0, 0};
  SampleIterator<uint8_t>       out(output, 0, bps);
  const unsigned int            mask = (1 << bps) - 1;

  for(auto i = 0; i < it.samplesPerBase; i++, it++, out++)
  {
    EXPECT_EQ((testData[0] >> (8 - bps * (i + 1))) & mask, *it);
    EXPECT_EQ((testData[0] >> (8 - bps * (i + 1))) & mask, it.get());
    out.set(*it);
  }
  for(auto i = 0; i < it.samplesPerBase; i++, it++, out++)
  {
    EXPECT_EQ((testData[1] >> (8 - bps * (i + 1))) & mask, *it);
    EXPECT_EQ((testData[1] >> (8 - bps * (i + 1))) & mask, it.get());
    out.set(*it);
  }

  EXPECT_EQ(0, memcmp(testData, output, 2));
}

TEST(SampleIterator_Tests, equality) // NOLINT
{
  EXPECT_EQ(SampleIterator<uint8_t>(nullptr, 0, 1), SampleIterator<uint8_t>(nullptr, 0, 1));
  EXPECT_NE(SampleIterator<const uint8_t>(nullptr, 0, 1), SampleIterator<const uint8_t>(testData, 0, 1));
  EXPECT_NE(SampleIterator<uint8_t>(nullptr, 0, 1), SampleIterator<uint8_t>(nullptr, 1, 1));
  EXPECT_NE(SampleIterator<uint8_t>(nullptr, 0, 1), SampleIterator<uint8_t>(nullptr, 0, 2));
}

using ArithmeticParam = std::tuple<int, int, int>;

class SampleIterator_arithmetic : public ::testing::TestWithParam<ArithmeticParam>
{
};

INSTANTIATE_TEST_SUITE_P(SampleIterator_Tests,
                         SampleIterator_arithmetic,
                         ::testing::Combine(::testing::ValuesIn(bit_depths),
                                            ::testing::ValuesIn(initial_offsets),
                                            ::testing::ValuesIn(deltas)));

TEST_P(SampleIterator_arithmetic, Test) // NOLINT
{
  auto [bps, initial_offset, delta] = GetParam();

  const SampleIterator<const uint8_t> start(nullptr, initial_offset, bps);
  SampleIterator<const uint8_t>       expected = start;
  for(auto i = 0; i < delta; i++, expected++)
  {
  }

  SampleIterator<const uint8_t> result = start;
  result += delta;

  EXPECT_EQ(result, expected);
  EXPECT_EQ(start + delta, expected);
  EXPECT_EQ(delta + start, expected);
}
