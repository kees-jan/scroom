/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <fmt/format.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <scroom/showmetadata.hh>
#include <scroom/unused.hh>

namespace Scroom::Metadata
{
  namespace
  {
    struct MetaDataWindowData
    {
      std::string title;
      Metadata    data;
    };

    GtkWidget* gtk_label_with_markup(const char* text)
    {
      GtkWidget* label = gtk_label_new(text);
      gtk_label_set_use_markup(GTK_LABEL(label), true);
      return label;
    }

    GtkWidget* addNewKeyToGrid(const GtkWidget* grid, GtkWidget* previousKey, const std::string& keyText)
    {
      GtkWidget* key = gtk_label_with_markup(keyText.c_str());
      if(previousKey)
      {
        gtk_grid_attach_next_to(GTK_GRID(grid), key, previousKey, GTK_POS_BOTTOM, 3, 3);
      }
      else
      {
        gtk_grid_attach(GTK_GRID(grid), key, 0, GTK_POS_RIGHT, 3, 3);
      }
      return key;
    }

    GtkWidget* addNewValueToGrid(const GtkWidget* grid, const std::string& valueText, GtkWidget* key)
    {
      GtkWidget* value = gtk_label_new(valueText.c_str());
      gtk_grid_attach_next_to(GTK_GRID(grid), value, key, GTK_POS_RIGHT, 3, 3);
      return value;
    }

    void addKeyAndValueToSizeGroup(GtkWidget* key, GtkWidget* value)
    {
      auto* group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
      gtk_size_group_add_widget(group, key);
      gtk_size_group_add_widget(group, value);
      g_object_unref(group);
    }

    void on_metadata_destroyed(GtkWidget*, gpointer user_data)
    {
      delete static_cast<const MetaDataWindowData*>(user_data); // NOLINT(cppcoreguidelines-owning-memory)
    }
  } // namespace

  void showMetaData(GtkWindow* parent, std::string title, Metadata data)
  {
    for(auto& [key, _]: data)
    {
      UNUSED(_);
      key = fmt::format("<b>{}:</b>", key);
    }
    const auto* windowData = new MetaDataWindowData{std::move(title), std::move(data)}; // NOLINT(cppcoreguidelines-owning-memory)

    // Create properties window
    auto* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    g_signal_connect(static_cast<gpointer>(window),
                     "destroy",
                     G_CALLBACK(on_metadata_destroyed),
                     const_cast<MetaDataWindowData*>(windowData)); // NOLINT(cppcoreguidelines-pro-type-const-cast)

    gtk_window_set_title(GTK_WINDOW(window), windowData->title.c_str());
    auto* grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget* previousKey = nullptr;
    for(const auto& [keyText, valueText]: windowData->data)
    {
      GtkWidget* key   = addNewKeyToGrid(grid, previousKey, keyText);
      GtkWidget* value = addNewValueToGrid(grid, valueText, key);
      addKeyAndValueToSizeGroup(key, value);

      previousKey = key;
    }

    gtk_window_set_transient_for(GTK_WINDOW(window), parent);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show_all(window);
    gtk_widget_grab_focus(window);
  }
} // namespace Scroom::Metadata