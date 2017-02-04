/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#ifndef PROGRESSINTERFACEHELPERS_HH
#define PROGRESSINTERFACEHELPERS_HH

#include <set>

#include <boost/thread.hpp>

#include <scroom/stuff.hh>
#include <scroom/progressinterface.hh>
#include <scroom/utilities.hh>

namespace Scroom
{
  namespace Utils
  {
    /**
     * Alternative interface used for reporting progress information
     */
    class ProgressStateInterface
    {
    public:
      typedef boost::shared_ptr<ProgressStateInterface> Ptr;
      typedef boost::weak_ptr<ProgressStateInterface> WeakPtr;

      typedef enum
        {
          IDLE,
          WAITING,
          WORKING,
          FINISHED
        } State;

      virtual ~ProgressStateInterface() {}
  
      virtual void setProgress(State s, double progress=0.0)=0;
    };

    class ProgressInterfaceFromProgressStateInterface : public ProgressInterface, protected ProgressStateInterface
    {
    public:

      // ProgressInterface ///////////////////////////////////////////////////

      virtual void setIdle();
      virtual void setWaiting(double progress=0.0);
      virtual void setWorking(double progress);
      virtual void setFinished();
    };
    
    class ProgressInterfaceFromProgressStateInterfaceForwarder : public ProgressInterfaceFromProgressStateInterface
    {
    public:
      typedef boost::shared_ptr<ProgressInterfaceFromProgressStateInterfaceForwarder> Ptr;

    private:
      ProgressStateInterface::Ptr child;
      
    private:
      ProgressInterfaceFromProgressStateInterfaceForwarder(ProgressStateInterface::Ptr child);

    public:
      static Ptr create(ProgressStateInterface::Ptr child);

    protected:
      // ProgressStateInterface //////////////////////////////////////////////
      virtual void setProgress(State s, double progress=0.0);
    };

    class ProgressStateInterfaceFromProgressInterface : public ProgressStateInterface, protected ProgressInterface
    {
    public:
      virtual void setProgress(State s, double progress=0.0);
    };
    
    class ProgressStateInterfaceFromProgressInterfaceForwarder : public ProgressStateInterfaceFromProgressInterface
    {
    public:
      typedef boost::shared_ptr<ProgressStateInterfaceFromProgressInterfaceForwarder> Ptr;

    private:
      ProgressInterface::Ptr child;
      
    private:
      ProgressStateInterfaceFromProgressInterfaceForwarder(ProgressInterface::Ptr child);
      
    public:
      static Ptr create(ProgressInterface::Ptr child);

    protected:
      virtual void setIdle();
      virtual void setWaiting(double progress=0.0);
      virtual void setWorking(double progress);
      virtual void setFinished();
    };

    namespace Detail
    {
      class ProgressStore : public ProgressInterfaceFromProgressStateInterface
      {
      public:
        typedef boost::shared_ptr<ProgressStore> Ptr;

      private:
        State state;
        double progress;

      private:
        ProgressStore();
        
      public:
        static Ptr create();

        void init(ProgressInterface::Ptr const & i);
        
      protected:
        // ProgressStateInterface //////////////////////////////////////////////
        virtual void setProgress(State s, double progress=0.0);
      };
    }

    class ProgressInterfaceBroadcaster : virtual public Base, public ProgressInterface
    {
    public:
      typedef boost::shared_ptr<ProgressInterfaceBroadcaster> Ptr;

    private:
      class Unsubscriber
      {
      public:
        typedef boost::shared_ptr<Unsubscriber> Ptr;

      private:
        ProgressInterfaceBroadcaster::Ptr parent;
        ProgressInterface::Ptr child;
        
      private:
        Unsubscriber(ProgressInterfaceBroadcaster::Ptr const& parent, ProgressInterface::Ptr const& child);
        
      public:
        static Ptr create(ProgressInterfaceBroadcaster::Ptr const& parent, ProgressInterface::Ptr const& child);
        ~Unsubscriber();
      };

      friend class Unsubscriber;
      
    private:
      boost::mutex mut;
      std::set<ProgressInterface::Ptr> children;
      Detail::ProgressStore::Ptr store;

    public:
      static Ptr create();

    private:
      ProgressInterfaceBroadcaster();

      void unsubscribe(ProgressInterface::Ptr const& child);

    public:
      Stuff subscribe(ProgressInterface::Ptr const& child);

      // ProgressInterface ///////////////////////////////////////////////////
      
      virtual void setIdle();
      virtual void setWaiting(double progress=0.0);
      virtual void setWorking(double progress);
      virtual void setFinished();
    };

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
