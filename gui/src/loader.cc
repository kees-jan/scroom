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

#include "loader.hh"

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

void load(const std::string& filename)
{
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
