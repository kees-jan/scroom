/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <list>
#include <memory>
#include <string>

#include <gtk/gtk.h>

#include <scroom/bookkeeping.hh>
#include <scroom/interface.hh>
#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>

class ScroomInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<ScroomInterface>;

public:
  virtual PresentationInterface::Ptr newPresentation(const std::string& name)                                                 = 0;
  virtual Aggregate::Ptr             newAggregate(const std::string& name)                                                    = 0;
  virtual PresentationInterface::Ptr loadPresentation(const std::string& name, const std::string& relativeTo = std::string()) = 0;

  virtual void showPresentation(PresentationInterface::Ptr const& presentation) = 0;
};

class NewPresentationInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<NewPresentationInterface>;

public:
  virtual PresentationInterface::Ptr createNew() = 0;
};

class NewAggregateInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<NewAggregateInterface>;

public:
  virtual Aggregate::Ptr createNew() = 0;
};

class OpenPresentationInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<OpenPresentationInterface>;

public:
  virtual std::list<GtkFileFilter*> getFilters() = 0;

  virtual PresentationInterface::Ptr open(const std::string& fileName) = 0;
};

namespace Scroom::TiledBitmap
{
  class OpenTiledBitmapInterface;
} // namespace Scroom::TiledBitmap

class OpenInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<OpenInterface>;

public:
  virtual std::list<GtkFileFilter*> getFilters() = 0;

  virtual void open(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface) = 0;
};

class PresentationObserver : private Interface
{
public:
  using Ptr = std::shared_ptr<PresentationObserver>;

public:
  virtual void presentationAdded(PresentationInterface::Ptr p) = 0;
  virtual void presentationDeleted()                           = 0;
};

class ViewObserver : private Interface
{
public:
  using Ptr = std::shared_ptr<ViewObserver>;

public:
  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v) = 0;
};

class ScroomPluginInterface : private Interface
{
public:
  using Ptr = std::shared_ptr<ScroomPluginInterface>;

public:
  virtual void registerNewPresentationInterface(const std::string&            identifier,
                                                NewPresentationInterface::Ptr newPresentationInterface)                       = 0;
  virtual void registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface) = 0;
  virtual void registerOpenPresentationInterface(const std::string&             identifier,
                                                 OpenPresentationInterface::Ptr openPresentationInterface)                    = 0;
  virtual void
               registerOpenTiledBitmapInterface(const std::string&                                             identifier,
                                                std::shared_ptr<Scroom::TiledBitmap::OpenTiledBitmapInterface> openTiledBitmapInterface) = 0;
  virtual void registerOpenInterface(const std::string& identifier, OpenInterface::Ptr openInterface)          = 0;
  virtual void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer)                 = 0;
  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer) = 0;
};
