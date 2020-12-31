/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>

#include <scroom/presentationinterface.hh>

#include "sizedeterminer.hh"
#include "transparentoverlayviewinfo.hh"

class TransparentOverlayPresentation
  : public PresentationBase
  , public Aggregate
{
public:
  typedef boost::shared_ptr<TransparentOverlayPresentation> Ptr;

private:
  typedef std::map<ViewInterface::WeakPtr, TransparentOverlayViewInfo::Ptr> ViewDataMap;

private:
  std::list<PresentationInterface::Ptr> children;
  SizeDeterminer::Ptr                   sizeDeterminer;

  ViewDataMap viewData;

  TransparentOverlayPresentation();

public:
  static Ptr create();

  // PresentationInterface ///////////////////////////////////////////////
  Scroom::Utils::Rectangle<double> getRect() override;
  void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, Scroom::Utils::Rectangle<double> presentationArea, int zoom) override;
  bool getProperty(const std::string& name, std::string& value) override;
  bool isPropertyDefined(const std::string& name) override;
  std::string                      getTitle() override;
  void                             viewAdded(ViewInterface::WeakPtr vi) override;
  void                             viewRemoved(ViewInterface::WeakPtr vi) override;
  std::set<ViewInterface::WeakPtr> getViews() override;

  // Aggregate ///////////////////////////////////////////////////////////

  void addPresentation(PresentationInterface::Ptr const& p) override;

private:
  void setOptimalColor(PresentationInterface::Ptr const& p);
};
