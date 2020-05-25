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

class Listener : public SelectionListener, virtual public Scroom::Utils::Base{
public:
	Listener();

public:
	typedef boost::shared_ptr<Listener> Ptr;

public:
	static Ptr create();

public:
	virtual ~Listener();

	virtual void onSelection(Selection* measurement);
};

class Measure : public PluginInformationInterface, public ViewObserver, virtual public Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Measure> Ptr;

private:
  Measure();

public:
  static Ptr create();

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

  virtual ~Measure();
};

