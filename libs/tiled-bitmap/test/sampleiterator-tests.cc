/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <array>
#include <limits>
#include <ostream>

#include <gmock/gmock.h>

#include <scroom/bitmap-helpers.hh>

using Scroom::Bitmap::SampleIterator;
using testing::ElementsAre;

namespace Scroom::Bitmap
{
  template <typename Base>
  std::ostream& operator<<(std::ostream& os, const SampleIterator<Base>& iterator)
  {
    const int sampleInElement = iterator.samplesPerBase - 1 - iterator.currentOffset;
    return os << "(storage " << static_cast<const void*>(iterator.currentBase) << ", sample " << sampleInElement << ", "
              << iterator.bps << " bits)";
  }
} // namespace Scroom::Bitmap

namespace
{
  template <typename Storage, int SAMPLE_BITS>
  struct Packing
  {
    using Base = Storage;
    static constexpr int sampleBits = SAMPLE_BITS;
  };
} // namespace

// These are the packed widths used by bitmap operations. Full-width samples
// have a separate fixture because they occupy one whole storage element.
using PackedWidths = testing::Types<
  Packing<uint8_t, 1>,
  Packing<uint8_t, 2>,
  Packing<uint8_t, 4>,
  Packing<uint16_t, 1>,
  Packing<uint16_t, 2>,
  Packing<uint16_t, 4>,
  Packing<uint16_t, 8>>;

template <typename Format>
class PackedSampleIterator : public testing::Test
{
protected:
  // The underscore avoids GCC 13 -Wshadow errors for the test bodies' Base aliases.
  using Base_ = Format::Base;
  static constexpr int sampleBits = Format::sampleBits;
  static constexpr int storageBits = std::numeric_limits<Base_>::digits;
  static constexpr int samplesPerElement = storageBits / sampleBits;
  static constexpr Base_ maximumSample = (1U << sampleBits) - 1;
  static constexpr Base_ maximumElement = std::numeric_limits<Base_>::max();
  std::array<Base_, 3> storage{};
  SampleIterator<const Base_> iterator{storage.data(), 0, sampleBits};
};

TYPED_TEST_SUITE(PackedSampleIterator, PackedWidths);

TYPED_TEST(PackedSampleIterator, reads_the_first_and_last_samples_then_crosses_the_storage_boundary) // NOLINT
{
  using Base = TestFixture::Base_;
  const Base firstSample = 0;
  const Base lastSample = TestFixture::maximumSample;
  const Base secondSampleInNextElement = 1;
  // Put named samples in the first and last slots; intervening slots stay zero.
  this->storage[0] = static_cast<Base>((firstSample << (TestFixture::storageBits - TestFixture::sampleBits)) | lastSample);
  this->storage[1] = static_cast<Base>(secondSampleInNextElement << (TestFixture::storageBits - 2 * TestFixture::sampleBits));

  EXPECT_EQ(firstSample, this->iterator.get());
  this->iterator += TestFixture::samplesPerElement - 1;
  EXPECT_EQ(lastSample, *this->iterator++);
  EXPECT_EQ(0, *this->iterator);
  EXPECT_EQ(secondSampleInNextElement, *++this->iterator);
}

TYPED_TEST(PackedSampleIterator, overwriting_the_last_sample_preserves_the_prefix_and_next_element) // NOLINT
{
  using Base = TestFixture::Base_;
  this->storage.fill(TestFixture::maximumElement);
  const Base unchangedPrefix = TestFixture::maximumElement - TestFixture::maximumSample;
  const Base replacement = 0;
  SampleIterator<Base> lastSample(this->storage.data(), TestFixture::samplesPerElement - 1, TestFixture::sampleBits);

  lastSample.set(replacement);
  EXPECT_THAT(
    this->storage, ElementsAre(unchangedPrefix + replacement, TestFixture::maximumElement, TestFixture::maximumElement)
  );

  lastSample.set(TestFixture::maximumSample);
  EXPECT_THAT(this->storage, ElementsAre(TestFixture::maximumElement, TestFixture::maximumElement, TestFixture::maximumElement));
}

TYPED_TEST(PackedSampleIterator, addition_can_skip_a_whole_element_from_its_last_sample) // NOLINT
{
  using Base = TestFixture::Base_;
  const Base destinationValue = 1;
  this->storage[1] = TestFixture::maximumElement;
  this->storage[2] = static_cast<Base>(destinationValue << (TestFixture::storageBits - TestFixture::sampleBits));
  const SampleIterator<const Base> start(this->storage.data(), TestFixture::samplesPerElement - 1, TestFixture::sampleBits);
  const SampleIterator<const Base> destination(this->storage.data() + 2, 0, TestFixture::sampleBits);
  const unsigned oneWholeElementAndOneSample = TestFixture::samplesPerElement + 1;

  auto advanced = start;
  advanced += oneWholeElementAndOneSample;

  EXPECT_EQ(destination, advanced);
  EXPECT_EQ(destination, start + oneWholeElementAndOneSample);
  EXPECT_EQ(destination, oneWholeElementAndOneSample + start);
  EXPECT_EQ(destinationValue, *advanced);
  EXPECT_EQ(destinationValue, *(start + oneWholeElementAndOneSample));
  EXPECT_EQ(destinationValue, *(oneWholeElementAndOneSample + start));
  EXPECT_EQ(0, *++advanced);
}

template <typename Base>
class FullWidthSampleIterator : public testing::Test
{
protected:
  static constexpr int sampleBits = std::numeric_limits<Base>::digits;
  std::array<Base, 3> storage{};
  SampleIterator<const Base> iterator{storage.data(), 0, sampleBits};
};

using StorageTypes = testing::Types<uint8_t, uint16_t>;
TYPED_TEST_SUITE(FullWidthSampleIterator, StorageTypes);

TYPED_TEST(FullWidthSampleIterator, reads_whole_elements_without_losing_the_high_bits) // NOLINT
{
  const TypeParam firstValue = std::numeric_limits<TypeParam>::max() - 1;
  const TypeParam nextValue = std::numeric_limits<TypeParam>::max();
  this->storage[0] = firstValue;
  this->storage[1] = nextValue;

  EXPECT_EQ(firstValue, this->iterator.get());
  EXPECT_EQ(firstValue, *this->iterator++);
  EXPECT_EQ(nextValue, *this->iterator);
}

TYPED_TEST(FullWidthSampleIterator, writing_the_middle_sample_preserves_both_neighbours) // NOLINT
{
  const TypeParam before = 10;
  const TypeParam original = 20;
  const TypeParam after = 30;
  const TypeParam replacement = 40;
  this->storage = {before, original, after};
  SampleIterator<TypeParam> middle(this->storage.data(), 1, TestFixture::sampleBits);

  middle.set(replacement);

  EXPECT_THAT(this->storage, ElementsAre(before, replacement, after));
}

TYPED_TEST(FullWidthSampleIterator, addition_reaches_the_selected_element) // NOLINT
{
  const TypeParam destinationValue = 30;
  this->storage[2] = destinationValue;
  const unsigned destinationIndex = 2;
  auto advanced = this->iterator;

  advanced += destinationIndex;

  EXPECT_EQ(destinationValue, *advanced);
  EXPECT_EQ(destinationValue, *(this->iterator + destinationIndex));
  EXPECT_EQ(destinationValue, *(destinationIndex + this->iterator));
}

TYPED_TEST(FullWidthSampleIterator, equality_distinguishes_buffers_positions_and_sample_widths) // NOLINT
{
  const auto samePosition = this->iterator;
  const SampleIterator<const TypeParam> nextPosition(this->storage.data(), 1, TestFixture::sampleBits);
  const SampleIterator<const TypeParam> narrowerSample(this->storage.data(), 0, 1);
  const std::array<TypeParam, 3> anotherBuffer{};
  const SampleIterator<const TypeParam> otherBuffer(anotherBuffer.data(), 0, TestFixture::sampleBits);

  EXPECT_EQ(samePosition, this->iterator);
  EXPECT_NE(nextPosition, this->iterator);
  EXPECT_NE(narrowerSample, this->iterator);
  EXPECT_NE(otherBuffer, this->iterator);
}
