/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <stdio.h>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "colormapplugin.hh"

PluginInformationInterface::Ptr getPluginInformation()
{
  return Scroom::ColormapImpl::ColormapPlugin::create();
}
