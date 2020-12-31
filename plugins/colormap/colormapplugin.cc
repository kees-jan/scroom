/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "colormapplugin.hh"

#include <scroom/colormappable.hh>

#include "colormapprovider.hh"

////////////////////////////////////////////////////////////////////////

namespace Scroom
{
  namespace ColormapImpl
  {

    ColormapPlugin::ColormapPlugin() = default;

    ColormapPlugin::Ptr ColormapPlugin::create() { return Ptr(new ColormapPlugin()); }

    std::string ColormapPlugin::getPluginName() { return "Colormap"; }

    std::string ColormapPlugin::getPluginVersion() { return "0.0"; }

    void ColormapPlugin::registerCapabilities(ScroomPluginInterface::Ptr host)
    {
      host->registerPresentationObserver("Colormap", shared_from_this<ColormapPlugin>());
    }

    void ColormapPlugin::presentationAdded(PresentationInterface::Ptr p)
    {
      printf("ColormapPlugin: A presentation was created\n");
      if(p->isPropertyDefined(COLORMAPPABLE_PROPERTY_NAME))
      {
        printf("ColormapPlugin: It is colormappable!\n");
        ColormapProvider::Ptr cmp = ColormapProvider::create(p);
      }
    }

    void ColormapPlugin::presentationDeleted() { printf("ColormapPlugin: A presentation may have been deleted\n"); }

  } // namespace ColormapImpl
} // namespace Scroom
