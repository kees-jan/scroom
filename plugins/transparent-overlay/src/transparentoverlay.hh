/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class TransparentOverlay
  : public PluginInformationInterface
  , public NewAggregateInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = boost::shared_ptr<TransparentOverlay>;

private:
  TransparentOverlay() = default;

public:
  static Ptr create();

public:
  std::string getPluginName() override;
  std::string getPluginVersion() override;
  void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

  Aggregate::Ptr createNew() override;
};
