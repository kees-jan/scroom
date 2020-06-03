/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/bitmap-helpers.hh>

#include <boost/test/unit_test.hpp>
#include <boost/test/data/test_case.hpp>

namespace data = boost::unit_test::data;
using Scroom::Bitmap::SampleIterator;

namespace
{
  const uint8_t testData[] = { 0x65, 0xC7 };
  const int bit_depths[] = {1,2,4,8};
  const int initial_offsets[] = {0,1};
  const int deltas[] = {0,1,5};
}

namespace Scroom
{
  namespace Bitmap
  {
    template<typename T>
    std::ostream& operator<<(std::ostream& os, const SampleIterator<T>& it )
    {
      return os << '(' << static_cast<const void*>(it.currentBase) << ", " << it.currentOffset << ", " << it.bps << ')';
    }
  }
}


BOOST_AUTO_TEST_SUITE(SampleIterator_Tests)

BOOST_DATA_TEST_CASE(SampleIterator_iterates, data::make(bit_depths), bps)
{
  SampleIterator<const uint8_t> it(testData, 0, bps);
  uint8_t output[] = { 0,0 };
  SampleIterator<uint8_t> out(output, 0, bps);
  const unsigned int mask = (1<<bps) - 1;

  for(auto i=0; i<it.samplesPerBase; i++, it++, out++)
  {
    BOOST_CHECK_EQUAL((testData[0]>>(8-bps*(i+1)))&mask, *it);
    BOOST_CHECK_EQUAL((testData[0]>>(8-bps*(i+1)))&mask, it.get());
    out.set(*it);
  }
  for(auto i=0; i<it.samplesPerBase; i++, it++, out++)
  {
    BOOST_CHECK_EQUAL((testData[1]>>(8-bps*(i+1)))&mask, *it);
    BOOST_CHECK_EQUAL((testData[1]>>(8-bps*(i+1)))&mask, it.get());
    out.set(*it);
  }

  BOOST_CHECK_EQUAL(0, memcmp(testData, output, 2));
}

BOOST_AUTO_TEST_CASE(equality)
{
  BOOST_CHECK_EQUAL(SampleIterator<uint8_t>(0,0,1), SampleIterator<uint8_t>(0,0,1));
  BOOST_CHECK_NE(SampleIterator<const uint8_t>(0,0,1), SampleIterator<const uint8_t>(testData,0,1));
  BOOST_CHECK_NE(SampleIterator<uint8_t>(0,0,1), SampleIterator<uint8_t>(0,1,1));
  BOOST_CHECK_NE(SampleIterator<uint8_t>(0,0,1), SampleIterator<uint8_t>(0,0,2));
}

BOOST_DATA_TEST_CASE(arithmetic, data::make(bit_depths) * data::make(initial_offsets) * data::make(deltas), bps, initial_offset, delta)
{
  const SampleIterator<const uint8_t> start(0, initial_offset, bps);
  SampleIterator<const uint8_t> expected = start;
  for(auto i=0; i<delta; i++, expected++) {}

  SampleIterator<const uint8_t> result = start;
  result += delta;

  BOOST_CHECK_EQUAL(result, expected);
  BOOST_CHECK_EQUAL(start+delta, expected);
  BOOST_CHECK_EQUAL(delta+start, expected);
}


BOOST_AUTO_TEST_SUITE_END()
