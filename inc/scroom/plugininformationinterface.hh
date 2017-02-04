/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
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

#ifndef _PLUGININFORMATIONINTERFACE_H
#define _PLUGININFORMATIONINTERFACE_H

#include <boost/shared_ptr.hpp>

#include <scroom/scroominterface.hh>

#define PLUGIN_API_VERSION 2

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
