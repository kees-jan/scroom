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
