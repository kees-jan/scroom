/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <string>

#include <gtk/gtk.h>

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/utilities.hh>

class Tiff
  : public PluginInformationInterface
  , public OpenPresentationInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<Tiff>;

private:
  Tiff() = default;

public:
  static Ptr create();

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  std::string getPluginName() override;
  std::string getPluginVersion() override;
  void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

  ////////////////////////////////////////////////////////////////////////
  // OpenPresentationInterface

  std::list<GtkFileFilter*>  getFilters() override;
  PresentationInterface::Ptr open(const std::string& fileName) override;
};
