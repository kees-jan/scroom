/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>

namespace Scroom
{
  namespace ColormapImpl
  { /**
     * Register the ColormapPlugin, keep track of PresentationInterface instances
     */
    class ColormapPlugin
      : public PluginInformationInterface
      , public PresentationObserver
      , virtual public Scroom::Utils::Base
    {
    public:
      typedef boost::shared_ptr<ColormapPlugin> Ptr;

    private:
      std::list<PresentationInterface::WeakPtr> presentations;

    private:
      ColormapPlugin();

    public:
      static Ptr create();

    public:
      virtual std::string getPluginName();
      virtual std::string getPluginVersion();
      virtual void        registerCapabilities(ScroomPluginInterface::Ptr host);

      virtual void presentationAdded(PresentationInterface::Ptr p);
      virtual void presentationDeleted();
    };

  } // namespace ColormapImpl
} // namespace Scroom
