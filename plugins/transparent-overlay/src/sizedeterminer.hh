/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <map>
#include <memory>
#include <set>

#include <scroom/presentationinterface.hh>
#include <scroom/resizablepresentationinterface.hh>

class SizeDeterminer
{
public:
  using Ptr = std::shared_ptr<SizeDeterminer>;

private:
  class PresentationData
  {
  public:
    ResizablePresentationInterface::Ptr const         resizablePresentationInterface;
    Scroom::Utils::WeakKeySet<ViewInterface::WeakPtr> views;

  public:
    PresentationData(); // Don't use
    explicit PresentationData(ResizablePresentationInterface::Ptr resizablePresentationInterface);
  };

private:
  std::list<PresentationInterface::Ptr>                  presentations;
  std::map<PresentationInterface::Ptr, PresentationData> resizablePresentationData;

private:
  SizeDeterminer() = default;
  void sendUpdates();

public:
  static Ptr                                     create();
  void                                           add(PresentationInterface::Ptr const& p);
  [[nodiscard]] Scroom::Utils::Rectangle<double> getRect() const;

  void open(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi);
  void close(PresentationInterface::Ptr const& p, ViewInterface::WeakPtr const& vi);
};
