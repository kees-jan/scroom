/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "colormapplugin.hh"

#include <cstdio>

#include <spdlog/spdlog.h>

#include <scroom/colormappable.hh>

#include "colormapprovider.hh"
#include "version.h"

////////////////////////////////////////////////////////////////////////

namespace Scroom::ColormapImpl
{
  ColormapPlugin::Ptr ColormapPlugin::create() { return Ptr(new ColormapPlugin()); }

  std::string ColormapPlugin::getPluginName() { return "Colormap"; }

  std::string ColormapPlugin::getPluginVersion() { return PACKAGE_VERSION; }

  void ColormapPlugin::registerCapabilities(ScroomPluginInterface::Ptr host)
  {
    host->registerPresentationObserver("Colormap", shared_from_this<ColormapPlugin>());
  }

  void ColormapPlugin::presentationAdded(PresentationInterface::Ptr p)
  {
    spdlog::debug("ColormapPlugin: A presentation was created");
    if(p->isPropertyDefined(COLORMAPPABLE_PROPERTY_NAME))
    {
      spdlog::debug("ColormapPlugin: It is colormappable!");
      ColormapProvider::Ptr cmp = ColormapProvider::create(p);
    }
  }

  void ColormapPlugin::presentationDeleted() { spdlog::debug("ColormapPlugin: A presentation may have been deleted"); }

} // namespace Scroom::ColormapImpl
