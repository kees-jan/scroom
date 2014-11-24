/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2014 Kees-Jan Dijkzeul
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License, version 2, as published by the Free Software Foundation.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LOADER_HH
#define LOADER_HH

#include <gtk/gtk.h>

#include <scroom/scroominterface.hh>

void create(NewPresentationInterface* interface);
void load(const GtkFileFilterInfo& info);
void load(const std::string& filename);
PresentationInterface::Ptr loadPresentation(const GtkFileFilterInfo& info);
PresentationInterface::Ptr loadPresentation(const std::string& filename);
void destroyGtkFileFilterList(std::list<GtkFileFilter*>& l);

#endif
