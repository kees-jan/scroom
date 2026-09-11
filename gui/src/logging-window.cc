/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "logging-window.hh"

#include <array>
#include <utility>

#include <scroom/assertions.hh>

#include "callbacks.hh"

namespace
{
  constexpr gint64 PRESENT_COOLDOWN_US = 2LL * G_USEC_PER_SEC;

  void setTextTagStringProperty(GtkTextTag* textTag, const char* propertyName, const char* value)
  {
    GValue gvalue = G_VALUE_INIT;
    g_value_init(&gvalue, G_TYPE_STRING);
    g_value_set_static_string(&gvalue, value);
    g_object_set_property(G_OBJECT(textTag), propertyName, &gvalue);
    g_value_unset(&gvalue);
  }

  void setTextTagBooleanProperty(GtkTextTag* textTag, const char* propertyName, gboolean value)
  {
    GValue gvalue = G_VALUE_INIT;
    g_value_init(&gvalue, G_TYPE_BOOLEAN);
    g_value_set_boolean(&gvalue, value);
    g_object_set_property(G_OBJECT(textTag), propertyName, &gvalue);
    g_value_unset(&gvalue);
  }

  void setTextTagIntProperty(GtkTextTag* textTag, const char* propertyName, gint value)
  {
    GValue gvalue = G_VALUE_INIT;
    g_value_init(&gvalue, G_TYPE_INT);
    g_value_set_int(&gvalue, value);
    g_object_set_property(G_OBJECT(textTag), propertyName, &gvalue);
    g_value_unset(&gvalue);
  }

  GtkTextTag* addTextTag(GtkTextBuffer* textBuffer, const char* tagName)
  {
    auto* tagTable = gtk_text_buffer_get_tag_table(textBuffer);
    require(tagTable != nullptr);

    auto* textTag = gtk_text_tag_new(tagName);
    require(textTag != nullptr);
    gtk_text_tag_table_add(tagTable, textTag);

    return textTag;
  }

  void scrollTextViewToBottom(GtkTextView* textView)
  {
    auto* textBuffer = gtk_text_view_get_buffer(textView);
    if(textBuffer == nullptr)
    {
      return;
    }

    GtkTextIter endIter;
    gtk_text_buffer_get_end_iter(textBuffer, &endIter);
    GtkTextMark* endMark = gtk_text_buffer_create_mark(textBuffer, nullptr, &endIter, false);
    gtk_text_view_scroll_mark_onscreen(textView, endMark);
    gtk_text_buffer_delete_mark(textBuffer, endMark);

    auto* adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(textView));
    if(adjustment != nullptr)
    {
      const double bottom = gtk_adjustment_get_upper(adjustment) - gtk_adjustment_get_page_size(adjustment);
      gtk_adjustment_set_value(adjustment, bottom > 0.0 ? bottom : 0.0);
    }
  }

  const char* levelTagName(spdlog::level::level_enum level)
  {
    switch(level)
    {
    case spdlog::level::trace:
      return "log-trace";
    case spdlog::level::debug:
      return "log-debug";
    case spdlog::level::info:
      return "log-info";
    case spdlog::level::warn:
      return "log-warn";
    case spdlog::level::err:
      return "log-error";
    case spdlog::level::critical:
      return "log-critical";
    default:
      return "log-info";
    }
  }
} // namespace

LoggingWindow::LoggingWindow(Scroom::RingBufferSink::Ptr sink)
  : m_sink(std::move(sink))
  , m_maxDisplayedLines(m_sink->capacity())
{
}

LoggingWindow::Ptr LoggingWindow::create(Scroom::RingBufferSink::Ptr sink, const std::string& uiFileName)
{
  auto window = Ptr(new LoggingWindow(std::move(sink)));
  window->loadFromUiFile(uiFileName);
  window->initializeTags();

  auto snapshot = window->m_sink->snapshot();
  bool hasWarningOrHigher = false;
  for(const auto& entry: snapshot)
  {
    window->appendEntry(entry);
    hasWarningOrHigher = hasWarningOrHigher || entry.level >= spdlog::level::warn;
  }
  if(!snapshot.empty())
  {
    window->m_lastSeq = snapshot.back().seq;
  }
  window->trimToMaxLines();
  window->scrollToBottom();

  if(hasWarningOrHigher)
  {
    window->requestAttentionForWarningBatch();
  }

  return window;
}

LoggingWindow::~LoggingWindow()
{
  if(m_xml != nullptr)
  {
    g_object_unref(m_xml);
    m_xml = nullptr;
  }
}

void LoggingWindow::loadFromUiFile(const std::string& uiFileName)
{
  m_xml = gtk_builder_new();
  std::array<gchar*, 3> objectNames{};
  objectNames[0] = g_strdup("logging_window");
  objectNames[1] = g_strdup(MenuIds::MENUBAR_PROTOTYPE);
  objectNames[2] = nullptr;

  gtk_builder_add_objects_from_file(m_xml, uiFileName.c_str(), objectNames.data(), nullptr);
  g_free(objectNames[0]);
  g_free(objectNames[1]);

  m_window = GTK_WIDGET(gtk_builder_get_object(m_xml, "logging_window"));
  m_textView = GTK_TEXT_VIEW(gtk_builder_get_object(m_xml, "logging_textview"));
  GtkWidget* menubarContainer = GTK_WIDGET(gtk_builder_get_object(m_xml, "logging_menubar_container"));
  GtkWidget* menubar = GTK_WIDGET(gtk_builder_get_object(m_xml, MenuIds::MENUBAR_PROTOTYPE));

  require(m_window != nullptr);
  require(m_textView != nullptr);
  require(menubarContainer != nullptr);
  require(menubar != nullptr);

  gtk_container_add(GTK_CONTAINER(menubarContainer), menubar);
  gtk_widget_show(menubar);
  connect_logging_window_menu_callbacks(m_xml, m_window);

  m_textBuffer = gtk_text_view_get_buffer(m_textView);
  require(m_textBuffer != nullptr);

  gtk_window_set_title(GTK_WINDOW(m_window), "Scroom - Logs");

  g_signal_connect(static_cast<gpointer>(m_window), "delete-event", G_CALLBACK(onDeleteEvent), this);
  g_signal_connect(static_cast<gpointer>(m_window), "focus-in-event", G_CALLBACK(onFocusInEvent), this);
}

void LoggingWindow::initializeTags()
{
  auto* traceTag = addTextTag(m_textBuffer, "log-trace");
  setTextTagStringProperty(traceTag, "foreground", "#7f7f7f");
  setTextTagBooleanProperty(traceTag, "foreground-set", TRUE);

  auto* debugTag = addTextTag(m_textBuffer, "log-debug");
  setTextTagStringProperty(debugTag, "foreground", "#3b82f6");
  setTextTagBooleanProperty(debugTag, "foreground-set", TRUE);

  auto* infoTag = addTextTag(m_textBuffer, "log-info");
  setTextTagBooleanProperty(infoTag, "foreground-set", FALSE);

  auto* warnTag = addTextTag(m_textBuffer, "log-warn");
  setTextTagStringProperty(warnTag, "foreground", "#b8860b");
  setTextTagBooleanProperty(warnTag, "foreground-set", TRUE);
  setTextTagIntProperty(warnTag, "weight", PANGO_WEIGHT_BOLD);

  auto* errorTag = addTextTag(m_textBuffer, "log-error");
  setTextTagStringProperty(errorTag, "foreground", "#b91c1c");
  setTextTagBooleanProperty(errorTag, "foreground-set", TRUE);
  setTextTagIntProperty(errorTag, "weight", PANGO_WEIGHT_BOLD);

  auto* criticalTag = addTextTag(m_textBuffer, "log-critical");
  setTextTagStringProperty(criticalTag, "foreground", "#ffffff");
  setTextTagBooleanProperty(criticalTag, "foreground-set", TRUE);
  setTextTagStringProperty(criticalTag, "background", "#7f1d1d");
  setTextTagIntProperty(criticalTag, "weight", PANGO_WEIGHT_BOLD);
}

void LoggingWindow::appendEntry(const Scroom::RingBufferSink::Entry& entry)
{
  GtkTextIter insertEnd;
  gtk_text_buffer_get_end_iter(m_textBuffer, &insertEnd);

  // Preserve the insertion start across buffer mutation.
  GtkTextMark* startMark = gtk_text_buffer_create_mark(m_textBuffer, nullptr, &insertEnd, TRUE);
  gtk_text_buffer_insert(m_textBuffer, &insertEnd, entry.text.c_str(), -1);

  GtkTextIter begin;
  GtkTextIter end;
  gtk_text_buffer_get_iter_at_mark(m_textBuffer, &begin, startMark);
  gtk_text_buffer_get_end_iter(m_textBuffer, &end);
  gtk_text_buffer_apply_tag_by_name(m_textBuffer, levelTagName(entry.level), &begin, &end);
  gtk_text_buffer_delete_mark(m_textBuffer, startMark);
}

void LoggingWindow::trimToMaxLines()
{
  const int lineCount = gtk_text_buffer_get_line_count(m_textBuffer);
  const auto maxLines = static_cast<int>(m_maxDisplayedLines);

  if(lineCount <= maxLines)
  {
    return;
  }

  GtkTextIter begin;
  GtkTextIter trimEnd;
  gtk_text_buffer_get_start_iter(m_textBuffer, &begin);
  gtk_text_buffer_get_iter_at_line(m_textBuffer, &trimEnd, lineCount - maxLines);
  gtk_text_buffer_delete(m_textBuffer, &begin, &trimEnd);
}

bool LoggingWindow::isScrolledToBottom() const
{
  auto* adjustment = gtk_scrollable_get_vadjustment(GTK_SCROLLABLE(m_textView));
  if(adjustment == nullptr)
  {
    return true;
  }

  const double value = gtk_adjustment_get_value(adjustment);
  const double pageSize = gtk_adjustment_get_page_size(adjustment);
  const double upper = gtk_adjustment_get_upper(adjustment);
  constexpr double bottomSlackPixels = 16.0;
  return value + pageSize >= upper - bottomSlackPixels;
}

void LoggingWindow::scrollToBottom()
{
  scrollTextViewToBottom(m_textView);

  // Ensure we scroll after GTK has performed size allocation/layout updates.
  g_idle_add_full(
    G_PRIORITY_DEFAULT_IDLE,
    +[](gpointer userData) -> gboolean
    {
      auto* textView = GTK_TEXT_VIEW(userData);
      scrollTextViewToBottom(textView);

      return G_SOURCE_REMOVE;
    },
    g_object_ref(m_textView),
    +[](gpointer userData) { g_object_unref(userData); }
  );
}

void LoggingWindow::requestAttentionForWarningBatch()
{
  if(!gtk_widget_get_visible(m_window))
  {
    show();
    return;
  }

  gtk_window_set_urgency_hint(GTK_WINDOW(m_window), TRUE);

  const gint64 now = g_get_monotonic_time();
  if(now - m_lastPresentTimestampUs >= PRESENT_COOLDOWN_US)
  {
    gtk_window_present(GTK_WINDOW(m_window));
    m_lastPresentTimestampUs = now;
  }
}

gboolean LoggingWindow::onDeleteEvent(GtkWidget* widget, GdkEvent* /*deleteEvent*/, gpointer /*callbackData*/)
{
  gtk_widget_hide(widget);
  return TRUE;
}

gboolean LoggingWindow::onFocusInEvent(GtkWidget* /*widget*/, GdkEvent* /*focusEvent*/, gpointer userData)
{
  auto* loggingWindow = static_cast<LoggingWindow*>(userData);
  gtk_window_set_urgency_hint(GTK_WINDOW(loggingWindow->m_window), FALSE);
  return FALSE;
}

void LoggingWindow::consume()
{
  const bool stickToBottom = isScrolledToBottom();
  auto result = m_sink->consumeSince(m_lastSeq);
  m_lastSeq = result.nextSeq;

  bool hasWarningOrHigher = false;
  for(const auto& entry: result.entries)
  {
    appendEntry(entry);
    hasWarningOrHigher = hasWarningOrHigher || entry.level >= spdlog::level::warn;
  }

  trimToMaxLines();

  if(stickToBottom)
  {
    scrollToBottom();
  }

  if(hasWarningOrHigher)
  {
    requestAttentionForWarningBatch();
  }
}

void LoggingWindow::show()
{
  gtk_window_present(GTK_WINDOW(m_window));
  scrollToBottom();
  m_lastPresentTimestampUs = g_get_monotonic_time();
}
