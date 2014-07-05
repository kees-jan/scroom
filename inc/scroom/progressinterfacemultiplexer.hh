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

#include <set>

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
      class ChildData : public ProgressStateInterface
      {
      public:
        typedef boost::shared_ptr<ChildData> Ptr;

      public:
        boost::mutex mut;
        ProgressStateInterface::State state;
        double progress;
        
      private:
        ChildData();
        
      public:
        static Ptr create();

        void clearFinished();
        
        // ProgressStateInterface ///////////////////////////////////////////////////
        virtual void setProgress(State s, double progress=0.0);
      };
      
      class Child : public ProgressInterfaceFromProgressStateInterface
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
        ~Child();
        
        // ProgressStateInterface ///////////////////////////////////////////////////
        virtual void setProgress(State s, double progress=0.0);
      };

      friend class Child;
      
    private:
      boost::mutex mut;
      ProgressStateInterface::Ptr parent;
      std::set<ChildData::Ptr> children;
      
    private:
      ProgressInterfaceMultiplexer(ProgressInterface::Ptr parent);
      
    public:
      static Ptr create(ProgressInterface::Ptr parent);

      ProgressInterface::Ptr createProgressInterface();

    private:
      void updateProgressState();
      void unsubscribe(ChildData::Ptr data);
    };

  }
}


#endif
