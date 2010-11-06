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

#ifndef _PLUGININFORMATIONINTERFACE_H
#define _PLUGININFORMATIONINTERFACE_H

#include <scroominterface.hh>

#define PLUGIN_API_VERSION 0

class PluginInformationInterface
{
public:
  const int pluginApiVersion;

public:
  PluginInformationInterface()
    : pluginApiVersion(PLUGIN_API_VERSION)
  {}

  virtual std::string getPluginName()=0;
  virtual std::string getPluginVersion()=0;
  virtual void registerCapabilities(ScroomInterface* host)=0;
  virtual void unregisterCapabilities(ScroomInterface* host)=0;
  
};



#endif
