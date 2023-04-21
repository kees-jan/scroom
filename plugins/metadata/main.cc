/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/scroomplugin.hh>

#include "metadata.hh"

PluginInformationInterface::Ptr getPluginInformation() { return Metadata::create(); }
