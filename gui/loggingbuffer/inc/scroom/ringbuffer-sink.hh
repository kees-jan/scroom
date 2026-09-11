/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <spdlog/sinks/base_sink.h>
#include <spdlog/spdlog.h>

#include <boost/circular_buffer.hpp>

namespace Scroom
{
  class RingBufferSink : public spdlog::sinks::base_sink<std::mutex>
  {
  public:
    using Ptr = std::shared_ptr<RingBufferSink>;
    static constexpr std::size_t DEFAULT_CAPACITY = 5000;

    struct Entry
    {
      std::uint64_t seq;
      spdlog::level::level_enum level;
      std::string text;
    };

    struct ConsumeResult
    {
      std::vector<Entry> entries;
      std::uint64_t nextSeq = 0;
    };

    explicit RingBufferSink(std::function<void()> wakeupCallback, std::size_t capacity = DEFAULT_CAPACITY);

    [[nodiscard]] std::vector<Entry> snapshot();
    [[nodiscard]] ConsumeResult consumeSince(std::uint64_t lastSeq);

    [[nodiscard]] std::size_t capacity();

  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;
    void flush_() override;

  private:
    boost::circular_buffer<Entry> m_entries;
    std::uint64_t m_nextSeq = 1;
    bool m_pendingWakeup = false;
    std::function<void()> m_wakeupCallback;
  };

} // namespace Scroom
