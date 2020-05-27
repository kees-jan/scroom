/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gtk/gtk.h>

#include <string>
#include <list>

#include <boost/shared_ptr.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>
#include <scroom/bookkeeping.hh>

class ScroomInterface
{
public:
  typedef boost::shared_ptr<ScroomInterface> Ptr;

public:
  virtual ~ScroomInterface() {}

  virtual PresentationInterface::Ptr newPresentation(std::string const& name)=0;
  virtual Aggregate::Ptr newAggregate(std::string const& name)=0;
  virtual PresentationInterface::Ptr loadPresentation(std::string const& name, std::string const& relativeTo=std::string())=0;

  virtual void showPresentation(PresentationInterface::Ptr const& presentation)=0;
};

class NewPresentationInterface
{
public:
  typedef boost::shared_ptr<NewPresentationInterface> Ptr;

public:
  virtual ~NewPresentationInterface() {}

  virtual PresentationInterface::Ptr createNew()=0;
};

class NewAggregateInterface
{
public:
  typedef boost::shared_ptr<NewAggregateInterface> Ptr;

public:
  virtual ~NewAggregateInterface() {}

  virtual Aggregate::Ptr createNew()=0;
};

class OpenPresentationInterface
{
public:
  typedef boost::shared_ptr<OpenPresentationInterface> Ptr;

public:
  virtual ~OpenPresentationInterface() {}

  virtual std::list<GtkFileFilter*> getFilters()=0;

  virtual PresentationInterface::Ptr open(const std::string& fileName)=0;
};

class OpenInterface
{
public:
  typedef boost::shared_ptr<OpenInterface> Ptr;

public:
  virtual ~OpenInterface() {}

  virtual std::list<GtkFileFilter*> getFilters()=0;

  virtual void open(const std::string& fileName, ScroomInterface::Ptr const& scroomInterface)=0;
};

class PresentationObserver
{
public:
  typedef boost::shared_ptr<PresentationObserver> Ptr;

public:
  virtual ~PresentationObserver() {}

  virtual void presentationAdded(PresentationInterface::Ptr p)=0;
  virtual void presentationDeleted()=0;
};

class ViewObserver
{
public:
  typedef boost::shared_ptr<ViewObserver> Ptr;

public:
  virtual ~ViewObserver() {}

  virtual Scroom::Bookkeeping::Token viewAdded(ViewInterface::Ptr v)=0;
};

class ScroomPluginInterface
{
public:
  typedef boost::shared_ptr<ScroomPluginInterface> Ptr;

public:
  virtual ~ScroomPluginInterface() {}

  virtual void registerNewPresentationInterface(const std::string& identifier, NewPresentationInterface::Ptr newPresentationInterface)=0;
  virtual void registerNewAggregateInterface(const std::string& identifier, NewAggregateInterface::Ptr newAggregateInterface)=0;
  virtual void registerOpenPresentationInterface(const std::string& identifier, OpenPresentationInterface::Ptr openPresentationInterface)=0;
  virtual void registerOpenInterface(const std::string& identifier, OpenInterface::Ptr openInterface)=0;
  virtual void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer)=0;
  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer)=0;

  virtual PresentationInterface::Ptr loadPresentation(const std::string& filename)=0;
};

