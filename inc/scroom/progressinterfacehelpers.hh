/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <set>

#include <boost/thread.hpp>

#include <scroom/progressinterface.hh>
#include <scroom/stuff.hh>
#include <scroom/utilities.hh>

namespace Scroom
{
  namespace Utils
  {
    /**
     * Alternative interface used for reporting progress information
     */
    class ProgressStateInterface : private Interface
    {
    public:
      using Ptr     = boost::shared_ptr<ProgressStateInterface>;
      using WeakPtr = boost::weak_ptr<ProgressStateInterface>;

      enum State
      {
        IDLE,
        WAITING,
        WORKING,
        FINISHED
      };

      virtual void setProgress(State s, double progress = 0.0) = 0;
    };

    class ProgressInterfaceFromProgressStateInterface
      : public ProgressInterface
      , protected ProgressStateInterface
    {
    public:
      // ProgressInterface ///////////////////////////////////////////////////

      void setIdle() override;
      void setWaiting(double progress = 0.0) override;
      void setWorking(double progress) override;
      void setFinished() override;
    };

    class ProgressInterfaceFromProgressStateInterfaceForwarder : public ProgressInterfaceFromProgressStateInterface
    {
    public:
      using Ptr = boost::shared_ptr<ProgressInterfaceFromProgressStateInterfaceForwarder>;

    private:
      ProgressStateInterface::Ptr child;

    private:
      ProgressInterfaceFromProgressStateInterfaceForwarder(ProgressStateInterface::Ptr child);

    public:
      static Ptr create(ProgressStateInterface::Ptr child);

    protected:
      // ProgressStateInterface //////////////////////////////////////////////
      void setProgress(State s, double progress = 0.0) override;
    };

    class ProgressStateInterfaceFromProgressInterface
      : public ProgressStateInterface
      , protected ProgressInterface
    {
    public:
      void setProgress(State s, double progress = 0.0) override;
    };

    class ProgressStateInterfaceFromProgressInterfaceForwarder : public ProgressStateInterfaceFromProgressInterface
    {
    public:
      using Ptr = boost::shared_ptr<ProgressStateInterfaceFromProgressInterfaceForwarder>;

    private:
      ProgressInterface::Ptr child;

    private:
      ProgressStateInterfaceFromProgressInterfaceForwarder(ProgressInterface::Ptr child);

    public:
      static Ptr create(ProgressInterface::Ptr child);

    protected:
      void setIdle() override;
      void setWaiting(double progress = 0.0) override;
      void setWorking(double progress) override;
      void setFinished() override;
    };

    namespace Detail
    {
      class ProgressStore : public ProgressInterfaceFromProgressStateInterface
      {
      public:
        using Ptr = boost::shared_ptr<ProgressStore>;

      private:
        State  state{IDLE};
        double progress{0.0};

      private:
        ProgressStore() = default;

      public:
        static Ptr create();

        void init(ProgressInterface::Ptr const& i);

      protected:
        // ProgressStateInterface //////////////////////////////////////////////
        void setProgress(State s, double progress = 0.0) override;
      };
    } // namespace Detail

    class ProgressInterfaceBroadcaster
      : virtual public Base
      , public ProgressInterface
    {
    public:
      using Ptr = boost::shared_ptr<ProgressInterfaceBroadcaster>;

    private:
      class Unsubscriber
      {
      public:
        using Ptr = boost::shared_ptr<Unsubscriber>;

      private:
        ProgressInterfaceBroadcaster::Ptr parent;
        ProgressInterface::Ptr            child;

      private:
        Unsubscriber(ProgressInterfaceBroadcaster::Ptr const& parent, ProgressInterface::Ptr const& child);
        Unsubscriber(const Unsubscriber&)            = delete;
        Unsubscriber(Unsubscriber&&)                 = delete;
        Unsubscriber& operator=(const Unsubscriber&) = delete;
        Unsubscriber& operator=(Unsubscriber&&)      = delete;

      public:
        static Ptr create(ProgressInterfaceBroadcaster::Ptr const& parent, ProgressInterface::Ptr const& child);
        ~Unsubscriber();
      };

      friend class Unsubscriber;

    private:
      boost::mutex                     mut;
      std::set<ProgressInterface::Ptr> children;
      Detail::ProgressStore::Ptr       store;

    public:
      static Ptr create();

    private:
      ProgressInterfaceBroadcaster();

      void unsubscribe(ProgressInterface::Ptr const& child);

    public:
      Stuff subscribe(ProgressInterface::Ptr const& child);

      // ProgressInterface ///////////////////////////////////////////////////

      void setIdle() override;
      void setWaiting(double progress = 0.0) override;
      void setWorking(double progress) override;
      void setFinished() override;
    };

    class ProgressInterfaceMultiplexer : public virtual Base
    {
    public:
      using Ptr = boost::shared_ptr<ProgressInterfaceMultiplexer>;

    private:
      class ChildData : public ProgressStateInterface
      {
      public:
        using Ptr = boost::shared_ptr<ChildData>;

      public:
        boost::mutex                  mut;
        ProgressStateInterface::State state{ProgressStateInterface::IDLE};
        double                        progress{0.0};

      private:
        ChildData() = default;

      public:
        static Ptr create();

        void clearFinished();

        // ProgressStateInterface ///////////////////////////////////////////////////
        void setProgress(State s, double progress = 0.0) override;
      };

      class Child : public ProgressInterfaceFromProgressStateInterface
      {
      public:
        using Ptr = boost::shared_ptr<Child>;

      private:
        ProgressInterfaceMultiplexer::Ptr parent;
        ChildData::Ptr                    data;

      private:
        Child(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data);

      public:
        static Ptr create(ProgressInterfaceMultiplexer::Ptr parent, ChildData::Ptr data);

        Child(const Child&)            = delete;
        Child(Child&&)                 = delete;
        Child& operator=(const Child&) = delete;
        Child& operator=(Child&&)      = delete;
        ~Child() override;

        // ProgressStateInterface ///////////////////////////////////////////////////
        void setProgress(State s, double progress = 0.0) override;
      };

      friend class Child;

    private:
      boost::mutex                mut;
      ProgressStateInterface::Ptr parent;
      std::set<ChildData::Ptr>    children;

    private:
      ProgressInterfaceMultiplexer(ProgressInterface::Ptr parent);

    public:
      static Ptr create(ProgressInterface::Ptr parent);

      ProgressInterface::Ptr createProgressInterface();

    private:
      void updateProgressState();
      void unsubscribe(ChildData::Ptr data);
    };
  } // namespace Utils
} // namespace Scroom
