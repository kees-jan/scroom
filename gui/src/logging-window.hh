/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include <gtk/gtk.h>

#include <scroom/ringbuffer-sink.hh>

class LoggingWindow : public std::enable_shared_from_this<LoggingWindow>
{
public:
  using Ptr = std::shared_ptr<LoggingWindow>;

private:
  Scroom::RingBufferSink::Ptr m_sink;
  GtkBuilder* m_xml{nullptr};
  GtkWidget* m_window{nullptr};
  GtkTextView* m_textView{nullptr};
  GtkTextBuffer* m_textBuffer{nullptr};
  std::uint64_t m_lastSeq{0};
  std::size_t m_maxDisplayedLines{0};
  gint64 m_lastPresentTimestampUs{0};

private:
  explicit LoggingWindow(Scroom::RingBufferSink::Ptr sink);

  void loadFromUiFile(const std::string& uiFileName);
  void initializeTags();
  void appendEntry(const Scroom::RingBufferSink::Entry& entry);
  void trimToMaxLines();
  [[nodiscard]] bool isScrolledToBottom() const;
  void scrollToBottom();
  void requestAttentionForWarningBatch();

  static gboolean onDeleteEvent(GtkWidget* widget, GdkEvent* deleteEvent, gpointer callbackData);
  static gboolean onFocusInEvent(GtkWidget* widget, GdkEvent* focusEvent, gpointer userData);

public:
  static Ptr create(Scroom::RingBufferSink::Ptr sink, const std::string& uiFileName);

  ~LoggingWindow();

  LoggingWindow(const LoggingWindow&) = delete;
  LoggingWindow(LoggingWindow&&) = delete;
  LoggingWindow& operator=(const LoggingWindow&) = delete;
  LoggingWindow& operator=(LoggingWindow&&) = delete;

  void consume();
  void show();
};
