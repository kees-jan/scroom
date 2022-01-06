/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <boost/test/unit_test.hpp>

#include <scroom/plugininformationinterface.hh>
#include <scroom/scroomplugin.hh>

BOOST_AUTO_TEST_SUITE(Plugin_tests)

BOOST_AUTO_TEST_CASE(get_plugin_information) { PluginInformationInterface::Ptr pi = getPluginInformation(); }

BOOST_AUTO_TEST_SUITE_END()
