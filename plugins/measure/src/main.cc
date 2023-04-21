/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <cstdio>

#include <scroom/scroomplugin.hh>

#include "measure.hh"

PluginInformationInterface::Ptr getPluginInformation() { return Measure::create(); }
