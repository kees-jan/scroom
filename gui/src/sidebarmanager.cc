/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#include "sidebarmanager.hh"

SidebarManager::SidebarManager()
  : panelWindow(NULL), panel(NULL), widgets()
{
}

void SidebarManager::setWidgets(GtkWidget* panelWindow, GtkBox* panel)
{
  this->panelWindow = panelWindow;
  this->panel = panel;
}

void SidebarManager::addSideWidget(std::string title, GtkWidget* w)
{
  GtkWidget* e = gtk_expander_new(title.c_str());
  gtk_expander_set_expanded(GTK_EXPANDER(e), true);
  gtk_widget_show(e);
  gtk_box_pack_start_defaults(panel, e);
  gtk_container_add(GTK_CONTAINER(e), w);
  gtk_widget_show(w);

  widgets[w]=e;

  gtk_widget_show(panelWindow);
}

void SidebarManager::removeSideWidget(GtkWidget* w)
{
  std::map<GtkWidget*, GtkWidget*>::iterator cur = widgets.find(w);
  if(cur==widgets.end())
  {
    printf("PANIC: Can't find the widget I'm supposed to remove\n");
    gtk_widget_destroy(w);
  }
  else
  {
    gtk_widget_destroy(cur->second);
    widgets.erase(cur);
  }
  if(widgets.empty())
    gtk_widget_hide(panelWindow);
  else
    gtk_widget_show(panelWindow);
}
