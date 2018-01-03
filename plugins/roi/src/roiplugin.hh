/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class RoiPlugin : public PluginInformationInterface, public OpenInterface, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<RoiPlugin> Ptr;

private:
  RoiPlugin();

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface
  
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  ////////////////////////////////////////////////////////////////////////
  // OpenInterface
  
  virtual std::list<GtkFileFilter*> getFilters();
  virtual void open(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface);
};


