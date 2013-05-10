/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2013 Kees-Jan Dijkzeul
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

#ifndef _COLORMAPPLUGIN_HH
#define _COLORMAPPLUGIN_HH

#include <scroom/plugininformationinterface.hh>
#include <scroom/utilities.hh>

namespace Scroom
{
  namespace ColormapImpl
  {/**
   * Register the ColormapPlugin, keep track of PresentationInterface instances
   */
    class ColormapPlugin: public PluginInformationInterface,
        public PresentationObserver,
        public Scroom::Utils::Base
    {
    public:
      typedef boost::shared_ptr<ColormapPlugin> Ptr;

    private:
      std::list<PresentationInterface::WeakPtr> presentations;

    private:
      ColormapPlugin();

    public:
      static Ptr create();
      virtual ~ColormapPlugin();

    public:
      virtual std::string getPluginName();
      virtual std::string getPluginVersion();
      virtual void registerCapabilities(ScroomInterface::Ptr host);
      virtual void unregisterCapabilities(ScroomInterface::Ptr host);

      virtual void presentationAdded(PresentationInterface::Ptr p);
      virtual void presentationDeleted();

    };

  }
}
#endif
