/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/interface.hh>
#include <scroom/scroominterface.hh>

#define PLUGIN_API_VERSION 4

class PluginInformationInterface : private Interface
{
public:
  typedef boost::shared_ptr<PluginInformationInterface> Ptr;

public:
  const int pluginApiVersion;

public:
  PluginInformationInterface()
    : pluginApiVersion(PLUGIN_API_VERSION)
  {}

  virtual std::string getPluginName()                                       = 0;
  virtual std::string getPluginVersion()                                    = 0;
  virtual void        registerCapabilities(ScroomPluginInterface::Ptr host) = 0;
};
