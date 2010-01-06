#include "loader.hh"

#include <stdio.h>

#include "callbacks.hh"
#include "pluginmanager.hh"

////////////////////////////////////////////////////////////////////////

gboolean next(gpointer data)
{
  if(data)
  {
    ((Loader*)data)->next();
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////


Loader::Loader()
  : currentlyLoading(false)
{
}

Loader::~Loader()
{
  if(currentlyLoading)
    printf("ERROR: Still loading!\n");
}

void Loader::load(const GtkFileFilterInfo& info)
{
  boost::mutex::scoped_lock lock(remainingFilesMutex);
  remainingFiles.push_back(info);
  if(!currentlyLoading)
    gdk_threads_add_idle(::next, this);
}

Loader& Loader::getInstance()
{
  static Loader instance;
  return instance;
}

void Loader::next()
{
  GtkFileFilterInfo info;
  bool infoAvailable = false;

  { // New scope for scoped lock
    boost::mutex::scoped_lock lock(remainingFilesMutex);
    if(!remainingFiles.empty())
    {
      info = remainingFiles.front();
      remainingFiles.pop_front();
      infoAvailable = true;
      currentlyLoading = true;
    }
  }

  if(infoAvailable)
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
          presentation = cur->first->open(info.filename, this);
          find_or_create_scroom(presentation);
          return;
        }
      }
      // Failed??? Try next one...
      fileOperationComplete();
    }
  }
}

void Loader::fileOperationComplete()
{
  boost::mutex::scoped_lock lock(remainingFilesMutex);
  currentlyLoading=false;
  gdk_threads_add_idle(::next, this);
}
