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

  GtkWidget* button = gtk_button_new_with_label("Some obscure default");
  addSideWidget("Default", button);

}

void SidebarManager::addSideWidget(std::string title, GtkWidget* w)
{
  gtk_box_pack_start_defaults(panel, w);
  gtk_widget_show(w);
}

void SidebarManager::removeSideWidget(GtkWidget* w)
{
}
