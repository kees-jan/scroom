/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <string>

#include <gtk/gtk.h>

#include <scroom/opentiledbitmapinterface.hh>
#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/scroominterface.hh>
#include <scroom/utilities.hh>

class Tiff
  : public PluginInformationInterface
  , public Scroom::TiledBitmap::OpenTiledBitmapInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = std::shared_ptr<Tiff>;

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
  // OpenTiledBitmapInterface

  std::list<GtkFileFilter*> getFilters() override;

  std::tuple<Scroom::TiledBitmap::BitmapMetaData, Layer::Ptr, Scroom::TiledBitmap::ReloadFunction>
    open(const std::string& fileName) override;
};
