/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gtk/gtk.h>

#include <list>
#include <string>
#include <map>

#include <scroom/scroomplugin.hh>
#include <scroom/scroominterface.hh>
#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>
#include "workinterface.hh"
#include "view.hh"

struct PluginInformation
{
  GModule* plugin;
  PluginInformationInterface::Ptr pluginInformation;

  PluginInformation(GModule* plugin_, PluginInformationInterface::Ptr pluginInformation_)
    : plugin(plugin_), pluginInformation(pluginInformation_)
  {
  }
};

class PluginManager : public WorkInterface, public ScroomPluginInterface, virtual public Scroom::Utils::Base
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
  std::map<NewPresentationInterface::Ptr, std::string> newPresentationInterfaces;
  std::map<std::string, NewAggregateInterface::Ptr> newAggregateInterfaces;
  std::map<OpenPresentationInterface::Ptr, std::string> openPresentationInterfaces;
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

  virtual void registerNewPresentationInterface(const std::string& identifier, NewPresentationInterface::Ptr newPresentationInterface);
  virtual void registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface);
  virtual void registerOpenPresentationInterface(const std::string& extension, OpenPresentationInterface::Ptr openPresentationInterface);
  virtual void registerOpenInterface(const std::string& extension, OpenInterface::Ptr openInterface);
  virtual void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer);
  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer);

  const std::map<NewPresentationInterface::Ptr, std::string>& getNewPresentationInterfaces();
  const std::map<std::string, NewAggregateInterface::Ptr>& getNewAggregateInterfaces();
  const std::map<OpenPresentationInterface::Ptr, std::string>& getOpenPresentationInterfaces();
  const std::map<OpenInterface::Ptr, std::string>& getOpenInterfaces();
  const std::map<ViewObserver::Ptr, std::string>& getViewObservers();
  const std::map<PresentationObserver::Ptr, std::string>& getPresentationObservers();

public:
  static PluginManager::Ptr getInstance();
};

void startPluginManager(bool devMode);

