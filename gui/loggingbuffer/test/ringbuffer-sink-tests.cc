/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

#include <gtest/gtest.h>

#include <scroom/ringbuffer-sink.hh>
#include <scroom/threadpool.hh>

namespace
{
  std::string withoutTrailingNewline(std::string text)
  {
    if(!text.empty() && text.back() == '\n')
    {
      text.pop_back();
      if(!text.empty() && text.back() == '\r')
      {
        text.pop_back();
      }
    }

    return text;
  }

  std::shared_ptr<spdlog::logger> createLogger(const Scroom::RingBufferSink::Ptr& sink, const std::string& name)
  {
    auto logger = std::make_shared<spdlog::logger>(name, sink);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%v");
    return logger;
  }
} // namespace

TEST(RingBufferSinkTests, keeps_only_last_entries_once_capacity_is_reached) // NOLINT
{
  auto sink = std::make_shared<Scroom::RingBufferSink>([] {}, 3);
  auto logger = createLogger(sink, "ringbuffer-eviction");

  logger->info("line-1");
  logger->info("line-2");
  logger->info("line-3");
  logger->info("line-4");

  auto entries = sink->snapshot();

  ASSERT_EQ(3, entries.size());
  EXPECT_EQ("line-2", withoutTrailingNewline(entries[0].text));
  EXPECT_EQ("line-3", withoutTrailingNewline(entries[1].text));
  EXPECT_EQ("line-4", withoutTrailingNewline(entries[2].text));
}

TEST(RingBufferSinkTests, sequence_numbers_are_monotonic_in_snapshot) // NOLINT
{
  auto sink = std::make_shared<Scroom::RingBufferSink>([] {}, 4);
  auto logger = createLogger(sink, "ringbuffer-seq");

  logger->debug("a");
  logger->debug("b");
  logger->debug("c");

  auto entries = sink->snapshot();

  ASSERT_EQ(3, entries.size());
  EXPECT_LT(entries[0].seq, entries[1].seq);
  EXPECT_LT(entries[1].seq, entries[2].seq);
}

TEST(RingBufferSinkTests, consume_since_handles_evicted_entries) // NOLINT
{
  auto sink = std::make_shared<Scroom::RingBufferSink>([] {}, 3);
  auto logger = createLogger(sink, "ringbuffer-consume");

  logger->info("line-1");
  logger->info("line-2");
  logger->info("line-3");
  logger->info("line-4");
  logger->info("line-5");

  std::uint64_t lastSeq = 1;
  auto result = sink->consumeSince(lastSeq);
  auto const& entries = result.entries;

  ASSERT_EQ(3, entries.size());
  EXPECT_EQ("line-3", withoutTrailingNewline(entries[0].text));
  EXPECT_EQ("line-4", withoutTrailingNewline(entries[1].text));
  EXPECT_EQ("line-5", withoutTrailingNewline(entries[2].text));
  EXPECT_EQ(entries.back().seq, result.nextSeq);
}

TEST(RingBufferSinkTests, wakeup_callback_fires_once_per_burst) // NOLINT
{
  constexpr int burstSize = 10;
  std::atomic<int> wakeupCalls = 0;
  auto sink = std::make_shared<Scroom::RingBufferSink>([&wakeupCalls] { ++wakeupCalls; }, 20);
  auto logger = createLogger(sink, "ringbuffer-wakeup");

  for(int i = 0; i < burstSize; ++i)
  {
    logger->warn("burst-a");
  }

  EXPECT_EQ(1, wakeupCalls.load());

  std::uint64_t lastSeq = 0;
  auto firstResult = sink->consumeSince(lastSeq);
  lastSeq = firstResult.nextSeq;

  for(int i = 0; i < burstSize; ++i)
  {
    logger->warn("burst-b");
  }

  EXPECT_EQ(2, wakeupCalls.load());
}

TEST(RingBufferSinkTests, concurrent_producers_and_consumer_keep_sequence_consistent) // NOLINT
{
  constexpr int threadCount = 4;
  constexpr int linesPerThread = 500;
  constexpr int totalLines = threadCount * linesPerThread;

  auto sink = std::make_shared<Scroom::RingBufferSink>([] {}, static_cast<std::size_t>(totalLines + 16));
  auto logger = createLogger(sink, "ringbuffer-concurrency");
  auto pool = ThreadPool::create(threadCount);

  std::vector<boost::unique_future<void>> producers;
  producers.reserve(threadCount);

  std::uint64_t lastSeq = 0;
  std::size_t consumedCount = 0;

  for(int t = 0; t < threadCount; ++t)
  {
    producers.push_back(pool->submit(
      [logger, t]
      {
        for(int i = 0; i < linesPerThread; ++i)
        {
          logger->info("thread-{}-line-{}", t, i);
        }
      }
    ));
  }

  for(auto& producer: producers)
  {
    producer.wait();
    auto result = sink->consumeSince(lastSeq);
    lastSeq = result.nextSeq;
    consumedCount += result.entries.size();
  }

  auto remaining = sink->consumeSince(lastSeq);
  consumedCount += remaining.entries.size();
  lastSeq = remaining.nextSeq;

  EXPECT_EQ(totalLines, static_cast<int>(consumedCount));
  EXPECT_EQ(totalLines, static_cast<int>(lastSeq));
}
