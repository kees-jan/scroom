/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include "sizedeterminer.hh"

#include <algorithm>

#include <scroom/gtk-helpers.hh>
#include <scroom/assertions.hh>

namespace
{
  Scroom::Utils::Rectangle<double> DetermineSize(std::list<PresentationInterface::Ptr> presentations)
  {
    double left = std::numeric_limits<double>::max();
    double top = std::numeric_limits<double>::max();
    double right = std::numeric_limits<double>::min();
    double bottom = std::numeric_limits<double>::min();

    for(PresentationInterface::Ptr const& p: presentations)
    {
      Scroom::Utils::Rectangle<double> rect = p->getRect();
      left = std::min(left, rect.getLeft());
      top = std::min(top, rect.getTop());
      right = std::max(right, rect.getRight());
      bottom = std::max(bottom, rect.getBottom());
    }
    return Scroom::Utils::Rectangle<double>(left, top, right-left, bottom-top);
  }

  template<typename K, typename V>
  std::list<K> keys(std::map<K,V> const& m)
  {
    std::list<K> k;
    for(auto const& p: m)
      k.push_back(p.first);

    return k;
  }
}

////////////////////////////////////////////////////////////////////////

SizeDeterminer::PresentationData::PresentationData()
{
  // Can't conjure a ResizablePresentationInterface::Ptr out of thin
  // air. Hence calling this constructor is not valid. However,
  // std::map requires that it is present.
  defect();
}

SizeDeterminer::PresentationData::PresentationData(ResizablePresentationInterface::Ptr const& resizablePresentationInterface_)
  : resizablePresentationInterface(resizablePresentationInterface_)
{}

////////////////////////////////////////////////////////////////////////

SizeDeterminer::Ptr SizeDeterminer::create()
{
  return Ptr(new SizeDeterminer());
}

SizeDeterminer::SizeDeterminer()
{
}

void SizeDeterminer::add(PresentationInterface::Ptr const& p)
{
  ResizablePresentationInterface::Ptr r = boost::dynamic_pointer_cast<ResizablePresentationInterface>(p);
  if(r)
  {
    resizablePresentationData.insert(std::make_pair(p,PresentationData(r)));
  }
  else
    presentations.push_back(p);

  sendUpdates();
}

Scroom::Utils::Rectangle<double> SizeDeterminer::getRect() const
{
  if(!presentations.empty())
  {
    return DetermineSize(presentations);
  }
  if(!resizablePresentationData.empty())
  {
    return DetermineSize(keys(resizablePresentationData));
  }
  return Scroom::Utils::Rectangle<double>();
}

void SizeDeterminer::open(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi)
{
  if(resizablePresentationData.count(p))
  {
    PresentationData& d = resizablePresentationData[p];
    require(!d.views.count(vi));
    d.views.insert(vi);

    d.resizablePresentationInterface->setRect(vi, getRect());
  }
  else
  {
    require(!boost::dynamic_pointer_cast<ResizablePresentationInterface>(p));
  }
}

void SizeDeterminer::close(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi)
{
  if(resizablePresentationData.count(p))
  {
    PresentationData& d = resizablePresentationData[p];
    require(d.views.count(vi));
    d.views.erase(vi);
  }
  else
  {
    require(!boost::dynamic_pointer_cast<ResizablePresentationInterface>(p));
  }
}

void SizeDeterminer::sendUpdates()
{
  Scroom::Utils::Rectangle<double> rect = getRect();

  for(auto const& data: resizablePresentationData)
    for(auto const& view: data.second.views)
      data.second.resizablePresentationInterface->setRect(view, rect);
}
