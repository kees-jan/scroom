/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2012 Kees-Jan Dijkzeul
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

#ifndef _SCROOMINTERFACE_HH
#define _SCROOMINTERFACE_HH

#include <gtk/gtk.h>

#include <string>
#include <list>

#include <boost/shared_ptr.hpp>

#include <scroom/presentationinterface.hh>
#include <scroom/viewinterface.hh>

class NewInterface
{
public:
  typedef boost::shared_ptr<NewInterface> Ptr;

public:
  virtual ~NewInterface() {}
  
  virtual PresentationInterface::Ptr createNew()=0;
};

class OpenInterface
{
public:
  typedef boost::shared_ptr<OpenInterface> Ptr;

public:
  virtual ~OpenInterface() {}

  virtual std::list<GtkFileFilter*> getFilters()=0;
  
  virtual PresentationInterface::Ptr open(const std::string& fileName)=0;
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

  virtual void viewAdded(ViewInterface::Ptr v)=0;
  virtual void viewDeleted(ViewInterface::Ptr v)=0;
};

class ScroomInterface
{
public:
  typedef boost::shared_ptr<ScroomInterface> Ptr;

public:
  virtual ~ScroomInterface() {}

  virtual void registerNewInterface(const std::string& identifier, NewInterface::Ptr newInterface)=0;

  virtual void registerOpenInterface(const std::string& identifier, OpenInterface::Ptr openInterface)=0;

  virtual void registerViewObserver(const std::string& identifier, ViewObserver::Ptr observer)=0;
  virtual void unregisterViewObserver(ViewObserver::Ptr observer)=0;

  virtual void registerPresentationObserver(const std::string& identifier, PresentationObserver::Ptr observer)=0;
  virtual void unregisterPresentationObserver(PresentationObserver::Ptr observer)=0;
};


#endif
