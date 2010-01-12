#include "loader.hh"

#include <stdio.h>

#include <threadpool.hh>

#include "callbacks.hh"
#include "pluginmanager.hh"

////////////////////////////////////////////////////////////////////////

void create(NewInterface* interface)
{
  PresentationInterface* presentation = interface->createNew(NULL);
  find_or_create_scroom(presentation);
  //  sequentially(new CreateOperation(interface));
}

void load(const GtkFileFilterInfo& info)
{
  const std::map<OpenInterface*, std::string>& openInterfaces = PluginManager::getInstance().getOpenInterfaces();
  PresentationInterface* presentation = NULL;
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
        find_or_create_scroom(presentation);
      }
    }
  }
  //  sequentially(new LoadOperation(info));
}


