/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gtk/gtk.h>

#include <scroom/scroominterface.hh>

void create(NewPresentationInterface* interface);
void load(const GtkFileFilterInfo& info);
void load(const std::string& filename);
PresentationInterface::Ptr loadPresentation(const GtkFileFilterInfo& info);
PresentationInterface::Ptr loadPresentation(const std::string& filename);
void destroyGtkFileFilterList(std::list<GtkFileFilter*>& l);


