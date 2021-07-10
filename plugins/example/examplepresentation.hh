/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <string>

#include <cairo.h>

#include <scroom/presentationinterface.hh>
#include <scroom/rectangle.hh>
#include <scroom/viewinterface.hh>

class ExamplePresentation : public PresentationInterface
{
private:
  cairo_pattern_t* pattern{nullptr};

  void fillPattern();

public:
  ExamplePresentation();
  ~ExamplePresentation() override;
  ExamplePresentation(const ExamplePresentation&) = delete;
  ExamplePresentation(ExamplePresentation&&)      = delete;
  ExamplePresentation operator=(const ExamplePresentation&) = delete;
  ExamplePresentation operator=(ExamplePresentation&&) = delete;

  Scroom::Utils::Rectangle<double> getRect() override;
  void                             open(ViewInterface::WeakPtr viewInterface) override;
  void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;
  void close(ViewInterface::WeakPtr vi) override;
  bool getProperty(const std::string& name, std::string& value) override;
  bool isPropertyDefined(const std::string& name) override;
  std::string getTitle() override;
  void        showMetadata() override;
};
