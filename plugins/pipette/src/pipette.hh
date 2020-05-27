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

class PipetteHandler : public PostRenderer, public SelectionListener, virtual public Scroom::Utils::Base
{
public:
  PipetteHandler();

public:
  typedef boost::shared_ptr<PipetteHandler> Ptr;

private:
  Selection* selection;

public:
  ViewInterface::Ptr view;

public:
  static Ptr create();

public:
  virtual ~PipetteHandler();

  virtual void render(cairo_t* cr);
  virtual void onSelectionStart(GdkPoint p);
  virtual void onSelectionUpdate(Selection* s);
  virtual void onSelectionEnd(Selection* s);
};

class Pipette : public PluginInformationInterface, public ViewObserver, virtual public  Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Pipette> Ptr;

private:
  Pipette();

public:
  static Ptr create();

public:
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v);

  virtual ~Pipette();
};
