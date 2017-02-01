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
