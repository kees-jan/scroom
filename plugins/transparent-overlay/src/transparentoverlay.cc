/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "transparentoverlay.hh"

#include <gdk/gdk.h>

#include "transparentoverlaypresentation.hh"
#include "version.h"

////////////////////////////////////////////////////////////////////////

TransparentOverlay::Ptr TransparentOverlay::create() { return Ptr(new TransparentOverlay()); }

std::string TransparentOverlay::getPluginName() { return "TransparentOverlay"; }

std::string TransparentOverlay::getPluginVersion() { return PACKAGE_VERSION; }

void TransparentOverlay::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerNewAggregateInterface("Transparent Overlay", shared_from_this<TransparentOverlay>());
}

Aggregate::Ptr TransparentOverlay::createNew() { return TransparentOverlayPresentation::create(); }
