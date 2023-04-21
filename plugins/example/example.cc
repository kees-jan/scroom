/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "example.hh"

#include <gdk/gdk.h>

#include "examplepresentation.hh"
#include "version.h"

////////////////////////////////////////////////////////////////////////

Example::Ptr Example::create() { return Ptr(new Example()); }

std::string Example::getPluginName() { return "Example"; }

std::string Example::getPluginVersion() { return PACKAGE_VERSION; }

void Example::registerCapabilities(ScroomPluginInterface::Ptr host)
{
  host->registerNewPresentationInterface("Example", shared_from_this<Example>());
}

PresentationInterface::Ptr Example::createNew() { return PresentationInterface::Ptr(new ExamplePresentation()); }
