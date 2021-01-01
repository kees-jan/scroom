/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class Tiff
  : public PluginInformationInterface
  , public OpenPresentationInterface
  , virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Tiff> Ptr;

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
