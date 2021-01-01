/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <string>

#include <gtk/gtk.h>

#include <scroom/plugininformationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/scroomplugin.hh>
#include <scroom/utilities.hh>

#include "view.hh"
#include "workinterface.hh"

struct PluginInformation
{
  GModule*                        plugin;
  PluginInformationInterface::Ptr pluginInformation;

  PluginInformation(GModule* plugin_, PluginInformationInterface::Ptr pluginInformation_)
    : plugin(plugin_)
    , pluginInformation(pluginInformation_)
  {}
};

class PluginManager
  : public WorkInterface
  , public ScroomPluginInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<PluginManager>;

private:
  enum PluginManagerState
  {
    FINDING_DIRECTORIES,
    SCANNING_DIRECTORIES,
    LOADING_FILES,
    DONE
  };

private:
  bool                                                  devMode{false};
  PluginManagerState                                    state{FINDING_DIRECTORIES};
  std::list<std::string>                                dirs;
  std::list<std::string>::iterator                      currentDir;
  std::list<std::string>                                files;
  std::list<std::string>::iterator                      currentFile;
  std::list<PluginInformation>                          pluginInformationList;
  std::map<NewPresentationInterface::Ptr, std::string>  newPresentationInterfaces;
  std::map<std::string, NewAggregateInterface::Ptr>     newAggregateInterfaces;
  std::map<OpenPresentationInterface::Ptr, std::string> openPresentationInterfaces;
  std::map<OpenInterface::Ptr, std::string>             openInterfaces;
  std::map<ViewObserver::Ptr, std::string>              viewObservers;
  std::map<PresentationObserver::Ptr, std::string>      presentationObservers;

private:
  void setStatusBarMessage(const char* message);

  PluginManager() = default;

public:
  static Ptr create();

  bool doWork() override;

  void addHook(bool devMode);

  void registerNewPresentationInterface(const std::string&            identifier,
                                        NewPresentationInterface::Ptr newPresentationInterface) override;
  void registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface) override;
  void registerOpenPresentationInterface(const std::string&             extension,
                                         OpenPresentationInterface::Ptr openPresentationInterface) override;
  void registerOpenInterface(const std::string& extension, OpenInterface::Ptr openInterface) override;
  void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer) override;
  void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer) override;

  const std::map<NewPresentationInterface::Ptr, std::string>&  getNewPresentationInterfaces();
  const std::map<std::string, NewAggregateInterface::Ptr>&     getNewAggregateInterfaces();
  const std::map<OpenPresentationInterface::Ptr, std::string>& getOpenPresentationInterfaces();
  const std::map<OpenInterface::Ptr, std::string>&             getOpenInterfaces();
  const std::map<ViewObserver::Ptr, std::string>&              getViewObservers();
  const std::map<PresentationObserver::Ptr, std::string>&      getPresentationObservers();

public:
  static PluginManager::Ptr getInstance();
};

void startPluginManager(bool devMode);
