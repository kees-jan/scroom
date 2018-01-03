/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _PLUGININFORMATIONINTERFACE_H
#define _PLUGININFORMATIONINTERFACE_H

#include <boost/shared_ptr.hpp>

#include <scroom/scroominterface.hh>

#define PLUGIN_API_VERSION 3

class PluginInformationInterface
{
public:
  typedef boost::shared_ptr<PluginInformationInterface> Ptr;

public:
  const int pluginApiVersion;

public:
  PluginInformationInterface()
    : pluginApiVersion(PLUGIN_API_VERSION)
  {}

  virtual ~PluginInformationInterface() {}

  virtual std::string getPluginName()=0;
  virtual std::string getPluginVersion()=0;
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host)=0;
};



#endif
