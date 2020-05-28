/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>
#include <scroom/viewinterface.hh>

class Pipette : public PostRenderer, public SelectionListener, public PluginInformationInterface, public ViewObserver, virtual public  Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Pipette> Ptr;

private:
  Pipette();

public:
  static Ptr create();

private:
  Selection* selection;
  bool enabled;
  ViewInterface::Ptr view;

public:
  ////////////////////////////////////////////////////////////////////////
  // PluginInformationInterface

  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  ////////////////////////////////////////////////////////////////////////
  // ViewObserver

  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v);

  ////////////////////////////////////////////////////////////////////////
  // PostRenderer

  virtual void render(cairo_t* cr);

  ////////////////////////////////////////////////////////////////////////
  // SelectionListener

  virtual void onSelectionStart(GdkPoint p);
  virtual void onSelectionUpdate(Selection* s);
  virtual void onSelectionEnd(Selection* s);

  ////////////////////////////////////////////////////////////////////////

  virtual ~Pipette();
};
