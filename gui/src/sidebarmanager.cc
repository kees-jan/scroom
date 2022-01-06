/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "sidebarmanager.hh"

#include <cstdio>
#include <utility>

#include <scroom/assertions.hh>

void SidebarManager::setWidgets(GtkWidget* panelWindow_, GtkBox* panel_)
{
  panelWindow = panelWindow_;
  panel       = panel_;
}

void SidebarManager::addSideWidget(std::string title, GtkWidget* w)
{
  GtkWidget* e = gtk_expander_new(title.c_str());
  gtk_expander_set_expanded(GTK_EXPANDER(e), true);
  gtk_widget_show(e);
  gtk_box_pack_start(panel, e, false, false, 0);
  gtk_container_add(GTK_CONTAINER(e), w);
  gtk_widget_show(w);

  widgets[w] = e;

  gtk_widget_show(panelWindow);
}

void SidebarManager::removeSideWidget(GtkWidget* w)
{
  auto cur = widgets.find(w);

  require(cur != widgets.end());

  gtk_widget_destroy(cur->second);
  widgets.erase(cur);

  if(widgets.empty())
  {
    gtk_widget_hide(panelWindow);
  }
  else
  {
    gtk_widget_show(panelWindow);
  }
}
