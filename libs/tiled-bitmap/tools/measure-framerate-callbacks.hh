/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <vector>

#include <boost/function.hpp>

#include <gtk/gtk.h>

extern std::vector<boost::function<bool()>> functions;

GtkWidget* create_window();
void       init();
void       invalidate();
