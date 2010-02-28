#include "sidebarmanager.hh"

SidebarManager::SidebarManager()
  : panelWindow(NULL), panel(NULL), widgets()
{
}

void SidebarManager::setWidgets(GtkWidget* panelWindow, GtkBox* panel)
{
  this->panelWindow = panelWindow;
  this->panel = panel;
  gtk_widget_show(panelWindow);
}

void SidebarManager::addSideWidget(std::string title, GtkWidget* w)
{
}

void SidebarManager::removeSideWidget(GtkWidget* w)
{
}
