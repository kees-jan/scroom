/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
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
      using Ptr = boost::shared_ptr<ColormapPlugin>;

    private:
      std::list<PresentationInterface::WeakPtr> presentations;

    private:
      ColormapPlugin() = default;

    public:
      static Ptr create();

    public:
      std::string getPluginName() override;
      std::string getPluginVersion() override;
      void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

      void presentationAdded(PresentationInterface::Ptr p) override;
      void presentationDeleted() override;
    };

  } // namespace ColormapImpl
} // namespace Scroom
