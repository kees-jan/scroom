/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdio>

#include <scroom/scroomplugin.hh>
#include <scroom/unused.hh>

#include "pipette.hh"

PluginInformationInterface::Ptr getPluginInformation() { return Pipette::create(); }
