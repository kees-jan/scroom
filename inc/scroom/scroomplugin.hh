/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

class PluginInformationInterface;

extern "C"
{
  using PluginFunc = boost::shared_ptr<PluginInformationInterface> (*)();

  boost::shared_ptr<PluginInformationInterface> getPluginInformation();
}
