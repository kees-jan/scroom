/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/presentationinterface.hh>

class ExamplePresentation : public PresentationInterface
{
private:
  cairo_pattern_t* pattern;

  void fillPattern();

public:
  ExamplePresentation();
  virtual ~ExamplePresentation();

  virtual Scroom::Utils::Rectangle<double> getRect();
  virtual void open(ViewInterface::WeakPtr viewInterface);
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom);
  virtual void close(ViewInterface::WeakPtr vi);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();
};

