/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef MEASURE_FRAMERATE_CALLBACKS_HH
#define MEASURE_FRAMERATE_CALLBACKS_HH

#include <vector>

#include <boost/function.hpp>

#include <gtk/gtk.h>

extern std::vector<boost::function<bool ()> > functions;

GtkWidget* create_window();
void init();
void invalidate();

#endif
