#ifndef LOADER_HH
#define LOADER_HH

#include <gtk/gtk.h>

#include <scroominterface.hh>

void create(NewInterface* interface);
void load(const GtkFileFilterInfo& info);
void load(const std::string& filename);

#endif
