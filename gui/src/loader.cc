/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "loader.hh"

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include <boost/filesystem.hpp>

#include <scroom/threadpool.hh>

#include "callbacks.hh"
#include "pluginmanager.hh"

////////////////////////////////////////////////////////////////////////

template<typename T>
class GObjectUnref
{
public:
  void operator()(T* p) { g_object_unref(p); }
};

class GtkFileFilterInfoDeleter
{
public:
  void operator()(GtkFileFilterInfo* f)
  {
    delete[] f->filename;
    delete[] f->mime_type;
    delete[] f->display_name;
    delete f;
  }
};

typedef std::unique_ptr<GtkFileFilterInfo,GtkFileFilterInfoDeleter> GtkFileFilterInfoPtr;

void destroyGtkFileFilterList(std::list<GtkFileFilter*>& l)
{
  while(!l.empty())
  {
    GtkFileFilter* f = l.front();
    g_object_ref_sink(f);
    g_object_unref(f);
    l.pop_front();
  }
}

class GtkFileFilterListDestroyer
{
private:
  std::list<GtkFileFilter*>& filters;

public:
  GtkFileFilterListDestroyer(std::list<GtkFileFilter*>& f) :
      filters(f)
  {
    for(auto const filter: filters)
      g_object_ref_sink(filter);
  }

  ~GtkFileFilterListDestroyer()
  {
    while(!filters.empty())
    {
      GtkFileFilter* f = filters.front();
      g_object_ref_sink(f);
      g_object_unref(f);
      filters.pop_front();
    }

  }
};

////////////////////////////////////////////////////////////////////////

char* charpFromString(std::string const& s)
{
  size_t n = s.size();
  std::unique_ptr<char[]> result(new char[n+1]);
  if(!result)
    throw std::bad_alloc();

  strncpy(result.get(), s.c_str(), n+1);
  if(result[n]!=0)
    throw std::length_error("String size changed during copying");

  return result.release();
}

GtkFileFilterInfoPtr filterInfoFromPath(const std::string& filename)
{
  GtkFileFilterInfoPtr filterInfo;

  std::unique_ptr<GFile,GObjectUnref<GFile> > file(g_file_new_for_path(filename.c_str()));
  std::unique_ptr<GFileInfo,GObjectUnref<GFileInfo> > fileInfo(g_file_query_info(file.get(), "standard::*", G_FILE_QUERY_INFO_NONE, NULL, NULL));
  if(fileInfo)
  {
    // g_file_info_get_name(fileInfo) doesn't provide path info.
    // charpFromString might throw.
    std::unique_ptr<gchar> filenameCopy(charpFromString(filename));
    std::unique_ptr<gchar> mime_type(charpFromString(g_content_type_get_mime_type(g_file_info_get_content_type (fileInfo.get()))));
    std::unique_ptr<gchar> display_name(charpFromString(g_file_info_get_display_name(fileInfo.get())));

    filterInfo.reset(new GtkFileFilterInfo()); // filterInfo->filename uninitialized, so a delete is dangerous

    filterInfo->filename = filenameCopy.release();
    filterInfo->mime_type = mime_type.release();
    filterInfo->display_name = display_name.release();
    filterInfo->contains =
      static_cast<GtkFileFilterFlags>(GTK_FILE_FILTER_FILENAME | GTK_FILE_FILTER_DISPLAY_NAME | GTK_FILE_FILTER_MIME_TYPE);
  }
  else
  {
    throw std::invalid_argument("No fileinfo for file "+filename);
  }

  return filterInfo;
}

bool filterMatchesInfo(GtkFileFilterInfo const& info, std::list<GtkFileFilter*> const& filters)
{
  for(auto const& f: filters)
    if(gtk_file_filter_filter(f, &info))
      return true;

  return false;
}

////////////////////////////////////////////////////////////////////////

class ScroomInterfaceImpl : public ScroomInterface
{
public:
  typedef boost::shared_ptr<ScroomInterfaceImpl> Ptr;

private:
  ScroomInterfaceImpl();

public:
  static Ptr instance();

  // ScroomInterface //////////////////////////////////////////////////////

  virtual PresentationInterface::Ptr newPresentation(std::string const& name);
  virtual Aggregate::Ptr newAggregate(std::string const& name);
  virtual PresentationInterface::Ptr loadPresentation(std::string const& name, std::string const& relativeTo=std::string());

  virtual void showPresentation(PresentationInterface::Ptr const& presentation);

};

////////////////////////////////////////////////////////////////////////

void create(NewPresentationInterface* interface)
{
  PresentationInterface::Ptr presentation = interface->createNew();
  if(presentation)
  {
    on_presentation_created(presentation);
    find_or_create_scroom(presentation);
  }
  else
    throw std::invalid_argument("Unable to create requested presentation");
}

PresentationInterface::Ptr loadPresentation(const std::string& filename)
{
  return loadPresentation(*filterInfoFromPath(filename));
}

PresentationInterface::Ptr loadPresentation(GtkFileFilterInfo const& info)
{
  static std::map<boost::filesystem::path,PresentationInterface::WeakPtr> loadedPresentations;
  // Create canonical path from filename
  const std::map<OpenPresentationInterface::Ptr, std::string>& openPresentationInterfaces = PluginManager::getInstance()->getOpenPresentationInterfaces();

#ifdef HAVE_BOOST_FILESYSTEM_CANONICAL
  boost::filesystem::path key(canonical(boost::filesystem::path(info.filename)));
#else
  boost::filesystem::path key(absolute(boost::filesystem::path(info.filename)));
#endif

  PresentationInterface::Ptr presentation = loadedPresentations[key].lock();

  if(!presentation)
  {
    for (auto const& cur : openPresentationInterfaces)
    {
      std::list<GtkFileFilter*> filters = cur.first->getFilters();
      if (filterMatchesInfo(info, filters))
      {
        presentation = cur.first->open(info.filename);

        if (presentation)
        {
          loadedPresentations[key] = presentation;
          on_presentation_created(presentation);
          break;
        }
      }
    }
  }

  if(presentation)
  {
    return presentation;
  }
  else
    throw std::invalid_argument("Don't know how to load presentation "+std::string(info.filename));
}

PresentationInterface::Ptr loadPresentation(GtkFileFilterInfoPtr const& info)
{
  return loadPresentation(*info);
}

void load(GtkFileFilterInfo const& info)
{
  const std::map<OpenPresentationInterface::Ptr, std::string>& openPresentationInterfaces = PluginManager::getInstance()->getOpenPresentationInterfaces();
  const std::map<OpenInterface::Ptr, std::string>& openInterfaces = PluginManager::getInstance()->getOpenInterfaces();

  for (auto const& cur : openInterfaces)
  {
    std::list<GtkFileFilter*> filters = cur.first->getFilters();
    GtkFileFilterListDestroyer destroyer(filters);
    if (filterMatchesInfo(info, filters))
    {
      cur.first->open(info.filename, ScroomInterfaceImpl::instance());
      return;
    }
  }
  for (auto const& cur : openPresentationInterfaces)
  {
    std::list<GtkFileFilter*> filters = cur.first->getFilters();
    GtkFileFilterListDestroyer destroyer(filters);
    if (filterMatchesInfo(info, filters))
    {
      PresentationInterface::Ptr presentation = cur.first->open(info.filename);
      if (presentation)
      {
        on_presentation_created(presentation);
        find_or_create_scroom(presentation);
        return;
      }
    }
  }

  throw std::invalid_argument("Don't know how to open file " + std::string(info.filename));
}

void load(GtkFileFilterInfoPtr const& info)
{
  load(*info);
}

void load(const std::string& filename)
{
  load(filterInfoFromPath(filename));
}

////////////////////////////////////////////////////////////////////////

ScroomInterfaceImpl::Ptr ScroomInterfaceImpl::instance()
{
  static ScroomInterfaceImpl::Ptr i(new ScroomInterfaceImpl());
  return i;
}

ScroomInterfaceImpl::ScroomInterfaceImpl()
{}

PresentationInterface::Ptr ScroomInterfaceImpl::newPresentation(std::string const& name)
{
  const std::map<NewPresentationInterface::Ptr, std::string>& newPresentationInterfaces = PluginManager::getInstance()->getNewPresentationInterfaces();

  for(auto const& p: newPresentationInterfaces)
  {
    if(p.second == name)
    {
      PresentationInterface::Ptr presentation = p.first->createNew();
      if(presentation)
      {
        on_presentation_created(presentation);
        return presentation;
      }
      else
      {
        throw std::invalid_argument("Failed to create a new "+name);
      }
    }
  }

  throw std::invalid_argument("Don't know how to create a new "+name);
}

Aggregate::Ptr ScroomInterfaceImpl::newAggregate(std::string const& name)
{
  std::map<std::string, NewAggregateInterface::Ptr> const& newAggregateInterfaces = PluginManager::getInstance()->getNewAggregateInterfaces();
  std::map<std::string, NewAggregateInterface::Ptr>::const_iterator i = newAggregateInterfaces.find(name);
  if(i != newAggregateInterfaces.end())
  {
    Aggregate::Ptr aggregate = i->second->createNew();
    if(aggregate)
    {
      PresentationInterface::Ptr aggregatePresentation =
          boost::dynamic_pointer_cast<PresentationInterface>(aggregate);

      if (aggregatePresentation)
        on_presentation_created(aggregatePresentation);

      return aggregate;
    }
    else
      throw std::invalid_argument("Failed to create a new"+name);
  }
  throw std::invalid_argument("Don't know how to create a new "+name);
}

PresentationInterface::Ptr ScroomInterfaceImpl::loadPresentation(std::string const& n, std::string const& rt)
{
  boost::filesystem::path name(n);
  boost::filesystem::path relativeTo(rt);

  if(!name.is_absolute() && relativeTo.has_parent_path())
  {
    name = relativeTo.parent_path() / name;
  }

  return ::loadPresentation(name.string());
}

void ScroomInterfaceImpl::showPresentation(PresentationInterface::Ptr const& presentation)
{
  find_or_create_scroom(presentation);
}

