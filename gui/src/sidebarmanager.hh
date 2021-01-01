/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <map>
#include <string>

#include <gtk/gtk.h>

class SidebarManager
{
private:
  GtkWidget* panelWindow{nullptr};
  GtkBox*    panel{nullptr};

  std::map<GtkWidget*, GtkWidget*> widgets;

public:
  void setWidgets(GtkWidget* panelWindow, GtkBox* panel);

  void addSideWidget(std::string title, GtkWidget* w);
  void removeSideWidget(GtkWidget* w);
};
