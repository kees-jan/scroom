/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class Tiff : public PluginInformationInterface, public OpenPresentationInterface, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Tiff> Ptr;

private:
  Tiff();

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  ////////////////////////////////////////////////////////////////////////
  // OpenPresentationInterface

  virtual std::list<GtkFileFilter*> getFilters();
  virtual PresentationInterface::Ptr open(const std::string& fileName);

  ////////////////////////////////////////////////////////////////////////

  virtual ~Tiff();
};

