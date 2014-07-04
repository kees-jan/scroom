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

#ifndef PROGRESSINTERFACEMULTIPLEXER_HH
#define PROGRESSINTERFACEMULTIPLEXER_HH

#include <boost/shared_ptr.hpp>

#include <scroom/progressinterface.hh>
#include <scroom/progressinterfacehelpers.hh>
#include <scroom/utilities.hh>

namespace Scroom
{
  namespace Utils
  {
    class ProgressInterfaceMultiplexer : public virtual Base
    {
    public:
      typedef boost::shared_ptr<ProgressInterfaceMultiplexer> Ptr;

    private:
      class ChildData
      {
      public:
        typedef boost::shared_ptr<ChildData> Ptr;

      public:
        ProgressStateInterface::State state;
        double progress;
        
      private:
        ChildData();
        
      public:
        static Ptr create();
      };
      
      class Child : public ProgressStateInterface
      {
      public:
        typedef boost::shared_ptr<Child> Ptr;

      private:
        ProgressInterfaceMultiplexer::Ptr parent;
        ChildData::Ptr data;
        
      private:
        Child(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data);
        
      public:
        static Ptr create(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data);
        
        // ProgressStateInterface ///////////////////////////////////////////////////
        virtual void setState(State s);
        virtual void setProgress(double d);
        virtual void setProgress(int done, int total);
      };

      friend class Child;
      
    private:
      ProgressStateInterface::Ptr parent;
      std::list<ChildData::Ptr> children;
      
    private:
      ProgressInterfaceMultiplexer(ProgressStateInterface::Ptr parent);
      
    public:
      static Ptr create(ProgressStateInterface::Ptr parent);

      ProgressStateInterface::Ptr createProgressInterface();

    private:
      void updateProgressState();
    };

  }
}


#endif
