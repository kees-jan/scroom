#include "loader.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <threadpool.hh>

#include "callbacks.hh"
#include "pluginmanager.hh"

////////////////////////////////////////////////////////////////////////

void create(NewInterface* interface)
{
  PresentationInterface::Ptr presentation = interface->createNew(NULL);
  on_presentation_created(presentation);
  find_or_create_scroom(presentation);
}

void load(const GtkFileFilterInfo& info)
{
  const std::map<OpenInterface*, std::string>& openInterfaces = PluginManager::getInstance().getOpenInterfaces();
  PresentationInterface::Ptr presentation;
  for(std::map<OpenInterface*, std::string>::const_iterator cur=openInterfaces.begin();
      cur != openInterfaces.end() && presentation==NULL;
      cur++)
  {
    std::list<GtkFileFilter*> filters = cur->first->getFilters();
    for(std::list<GtkFileFilter*>::iterator f = filters.begin();
        f != filters.end() && presentation==NULL;
        f++)
    {
      if(gtk_file_filter_filter(*f, &info))
      {
        presentation = cur->first->open(info.filename, NULL);
        on_presentation_created(presentation);
        find_or_create_scroom(presentation);
      }
    }
  }
}

#if MUTRACX_HACKS

void load(const std::string& filename)
{
  // Gtk 2.12
  GtkFileFilterInfo filterInfo;
  filterInfo.filename = filename.c_str();
  filterInfo.display_name = filename.c_str();
  filterInfo.contains =
    (GtkFileFilterFlags)(GTK_FILE_FILTER_FILENAME | GTK_FILE_FILTER_DISPLAY_NAME);
  
  printf("Opening file %s\n", filterInfo.filename);
  load(filterInfo);
}

#else

void load(const std::string& filename)
{
  // Gtk 2.16
  GFile* file = g_file_new_for_path(filename.c_str());
  GFileInfo* fileInfo = g_file_query_info(file, "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
  GtkFileFilterInfo filterInfo;
  filterInfo.filename = filename.c_str(); // g_file_info_get_name(fileInfo) doesn't provide path info.
  filterInfo.mime_type = g_content_type_get_mime_type(g_file_info_get_content_type (fileInfo));
  filterInfo.display_name = g_file_info_get_display_name(fileInfo);
  filterInfo.contains =
    (GtkFileFilterFlags)(GTK_FILE_FILTER_FILENAME | GTK_FILE_FILTER_DISPLAY_NAME | GTK_FILE_FILTER_MIME_TYPE);
  printf("Opening file %s (%s)\n", filterInfo.filename, filterInfo.mime_type);
  load(filterInfo);
}

#endif
