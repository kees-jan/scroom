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

#ifndef _SIZEDETERMINER_HH
#define _SIZEDETERMINER_HH

#include <list>
#include <set>
#include <map>

#include <boost/shared_ptr.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/resizablepresentationinterface.hh>

class SizeDeterminer
{
public:
  typedef boost::shared_ptr<SizeDeterminer> Ptr;

private:
  class PresentationData
  {
  public:
    ResizablePresentationInterface::Ptr const resizablePresentationInterface;
    std::set<ViewInterface::WeakPtr> views;
    
  public:
    PresentationData(); // Don't use
    explicit PresentationData(ResizablePresentationInterface::Ptr const& resizablePresentationInterface);
  };

private:
  std::list<PresentationInterface::Ptr> presentations;
  std::map<PresentationInterface::Ptr,PresentationData> resizablePresentationData;
  
private:
  SizeDeterminer();
  void sendUpdates();
  
public:
  static Ptr create();
  void add(PresentationInterface::Ptr const& p);
  GdkRectangle getRect() const;

  void open(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi);
  void close(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi);
};


#endif
