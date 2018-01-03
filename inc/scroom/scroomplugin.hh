/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _SCROOMPLUGIN_H
#define _SCROOMPLUGIN_H

#include <gmodule.h>

#include <boost/shared_ptr.hpp>

class PluginInformationInterface;

extern "C"
{
  typedef boost::shared_ptr<PluginInformationInterface> (*PluginFunc)();

  G_MODULE_IMPORT boost::shared_ptr<PluginInformationInterface> getPluginInformation();
}

#endif
