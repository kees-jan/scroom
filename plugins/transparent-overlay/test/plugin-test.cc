/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <gtest/gtest.h>

#include <scroom/plugininformationinterface.hh>
#include <scroom/scroomplugin.hh>

TEST(Plugin_tests, get_plugin_information) // NOLINT
{
  PluginInformationInterface::Ptr const pi = getPluginInformation();
}
