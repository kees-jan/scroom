/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2010 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef SIDEBARMANAGER_HH
#define SIDEBARMANAGER_HH

#include <map>
#include <string>

#include <gtk/gtk.h>

class SidebarManager
{
private:
  GtkWidget* panelWindow;
  GtkBox* panel;

  std::map<GtkWidget*, GtkWidget*> widgets;

public:
  SidebarManager();
  void setWidgets(GtkWidget* panelWindow, GtkBox* panel);

  void addSideWidget(std::string title, GtkWidget* w);
  void removeSideWidget(GtkWidget* w);
};

#endif
