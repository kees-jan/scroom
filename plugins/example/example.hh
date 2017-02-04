/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _EXAMPLE_HH
#define _EXAMPLE_HH

#include <boost/shared_ptr.hpp>

#include <scroom/plugininformationinterface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/utilities.hh>

class Example : public PluginInformationInterface, public NewPresentationInterface, virtual public  Scroom::Utils::Base
{
public:
  typedef boost::shared_ptr<Example> Ptr;

private:
  Example();

public:
  static Ptr create();

public:
  virtual std::string getPluginName();
  virtual std::string getPluginVersion();
  virtual void registerCapabilities(ScroomPluginInterface::Ptr host);

  virtual PresentationInterface::Ptr createNew();

  virtual ~Example();
};

#endif
