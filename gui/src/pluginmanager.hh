/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#ifndef _PLUGINMANAGER_HH
#define _PLUGINMANAGER_HH

#include <gtk/gtk.h>

#include <list>
#include <string>
#include <map>

#include <scroom/scroomplugin.hh>
#include <scroom/scroominterface.hh>
#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>
#include <workinterface.hh>
#include <view.hh>

struct PluginInformation
{
  GModule* plugin;
  PluginInformationInterface::Ptr pluginInformation;

  PluginInformation(GModule* plugin, PluginInformationInterface::Ptr pluginInformation)
    : plugin(plugin), pluginInformation(pluginInformation)
  {
  }
};

class PluginManager : public WorkInterface, public ScroomInterface, public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<PluginManager> Ptr;
private:
  typedef enum
    {
      FINDING_DIRECTORIES,
      SCANNING_DIRECTORIES,
      LOADING_FILES,
      DONE
    } PluginManagerState;
  
private:
  bool devMode;
  PluginManagerState state;
  std::list<std::string> dirs;
  std::list<std::string>::iterator currentDir;
  std::list<std::string> files;
  std::list<std::string>::iterator currentFile;
  std::list<PluginInformation> pluginInformationList;
  std::map<NewInterface::Ptr, std::string> newInterfaces;
  std::map<OpenInterface::Ptr, std::string> openInterfaces;
  std::map<ViewObserver::Ptr, std::string> viewObservers;
  std::map<PresentationObserver::Ptr, std::string> presentationObservers;

private:
  void setStatusBarMessage(const char* message);
  
  PluginManager();

public:

  static Ptr create();
  
  virtual bool doWork();

  void addHook(bool devMode);

  virtual void registerNewInterface(const std::string& identifier, NewInterface::Ptr newInterface);
  virtual void unregisterNewInterface(NewInterface::Ptr newInterface);

  virtual void registerOpenInterface(const std::string& extension, OpenInterface::Ptr openInterface);
  virtual void unregisterOpenInterface(OpenInterface::Ptr openInterface);

  virtual void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer);
  virtual void unregisterViewObserver(ViewObserver::Ptr observer);

  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer);
  virtual void unregisterPresentationObserver(PresentationObserver::Ptr observer);

  const std::map<NewInterface::Ptr, std::string>& getNewInterfaces();
  const std::map<OpenInterface::Ptr, std::string>& getOpenInterfaces();
  const std::map<ViewObserver::Ptr, std::string>& getViewObservers();
  const std::map<PresentationObserver::Ptr, std::string>& getPresentationObservers();

public:
  static PluginManager::Ptr getInstance();
};

void startPluginManager(bool devMode);

#endif
