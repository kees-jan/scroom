/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <utility>

#include <scroom/ringbuffer-sink.hh>

namespace Scroom
{
  RingBufferSink::RingBufferSink(std::function<void()> wakeupCallback, std::size_t capacity)
    : m_entries(capacity)
    , m_wakeupCallback(std::move(wakeupCallback))
  {
  }

  std::vector<RingBufferSink::Entry> RingBufferSink::snapshot()
  {
    std::lock_guard const lock(mutex_);
    return {m_entries.begin(), m_entries.end()};
  }

  RingBufferSink::ConsumeResult RingBufferSink::consumeSince(std::uint64_t lastSeq)
  {
    std::lock_guard const lock(mutex_);
    m_pendingWakeup = false;

    ConsumeResult result;
    result.nextSeq = lastSeq;
    result.entries.reserve(m_entries.size());

    for(auto const& entry: m_entries)
    {
      if(entry.seq > lastSeq)
      {
        result.entries.push_back(entry);
      }
    }

    if(!result.entries.empty())
    {
      result.nextSeq = result.entries.back().seq;
    }

    return result;
  }

  std::size_t RingBufferSink::capacity()
  {
    std::lock_guard const lock(mutex_);
    return m_entries.capacity();
  }

  void RingBufferSink::sink_it_(const spdlog::details::log_msg& msg)
  {
    spdlog::memory_buf_t formatted;
    formatter_->format(msg, formatted);

    m_entries.push_back(Entry{.seq = m_nextSeq++, .level = msg.level, .text = fmt::to_string(formatted)});

    if(m_pendingWakeup)
    {
      return;
    }

    m_pendingWakeup = true;

    if(m_wakeupCallback)
    {
      m_wakeupCallback();
    }
  }

  void RingBufferSink::flush_() {}

} // namespace Scroom
