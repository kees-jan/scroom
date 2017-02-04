/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef _TRANSPARENTOVERLAYPRESENTATION_HH
#define _TRANSPARENTOVERLAYPRESENTATION_HH

#include <list>
#include <map>

#include <scroom/presentationinterface.hh>

#include "transparentoverlayviewinfo.hh"
#include "sizedeterminer.hh"

class TransparentOverlayPresentation : public PresentationBase, public Aggregate
{
public:
  typedef boost::shared_ptr<TransparentOverlayPresentation> Ptr;

private:
  typedef std::map<ViewInterface::WeakPtr, TransparentOverlayViewInfo::Ptr> ViewDataMap;
private:
  std::list<PresentationInterface::Ptr> children;
  SizeDeterminer::Ptr sizeDeterminer;

  ViewDataMap viewData;
  
  TransparentOverlayPresentation();
public:
  static Ptr create();
  
  // PresentationInterface ///////////////////////////////////////////////
  virtual GdkRectangle getRect();
  virtual void redraw(ViewInterface::Ptr const& vi, cairo_t* cr, GdkRectangle presentationArea, int zoom);
  virtual bool getProperty(const std::string& name, std::string& value);
  virtual bool isPropertyDefined(const std::string& name);
  virtual std::string getTitle();
  virtual void viewAdded(ViewInterface::WeakPtr vi);
  virtual void viewRemoved(ViewInterface::WeakPtr vi);
  virtual std::set<ViewInterface::WeakPtr> getViews();


  // Aggregate ///////////////////////////////////////////////////////////

  virtual void addPresentation(PresentationInterface::Ptr const& p);

private:
  void setOptimalColor(PresentationInterface::Ptr const& p);
};

#endif
