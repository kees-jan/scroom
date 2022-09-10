/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class Example
  : public PluginInformationInterface
  , public NewPresentationInterface
  , virtual public Scroom::Utils::Base
{
public:
  using Ptr = std::shared_ptr<Example>;

private:
  Example() = default;

public:
  static Ptr create();

public:
  std::string getPluginName() override;
  std::string getPluginVersion() override;
  void        registerCapabilities(ScroomPluginInterface::Ptr host) override;

  PresentationInterface::Ptr createNew() override;
};
