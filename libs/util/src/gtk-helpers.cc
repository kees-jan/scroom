/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <scroom/gtk-helpers.hh>

#include <boost/thread.hpp>

#include <scroom/assertions.hh>

typedef boost::shared_ptr<boost::function<bool()> > GtkFuncPtr;

namespace Scroom
{
  namespace GtkHelpers
  {
    namespace Detail
    {
      class Wrapper
      {
      public:
        boost::function<bool()> f;

      public:
        static gpointer create(const boost::function<bool()>& f);
        Wrapper(const boost::function<bool()>& f);
      };

      Wrapper::Wrapper(const boost::function<bool()>& f_)
      :f(f_)
      {}

      gpointer Wrapper::create(const boost::function<bool()>& f)
      {
        return new Wrapper(f);
      }

      static int gtkWrapper(gpointer data)
      {
        Wrapper* w = reinterpret_cast<Wrapper*>(data);
        bool result = w->f();
        if(!result)
          delete w;
        return result;
      }
    }

    Wrapper::Wrapper(const boost::function<bool()>& f_)
        : f(&Detail::gtkWrapper), data(Detail::Wrapper::create(f_))
    {
    }

    Wrapper wrap(boost::function<bool()> f)
    {
      return Wrapper(f);
    }

    namespace Detail
    {
      boost::recursive_mutex& GdkMutex()
      {
        static boost::recursive_mutex me;
        return me;
      }

      void lockGdkMutex()
      {
        GdkMutex().lock();
      }

      void unlockGdkMutex()
      {
        GdkMutex().unlock();
      }
    }

    void useRecursiveGdkLock()
    {
      gdk_threads_set_lock_functions(&Detail::lockGdkMutex, &Detail::unlockGdkMutex);
    }

    TakeGdkLock::TakeGdkLock()
    {
      gdk_threads_enter();
    }

    TakeGdkLock::~TakeGdkLock()
    {
      gdk_threads_leave();
    }
  }
}

std::ostream& operator<<(std::ostream& os, GdkRectangle const& r)
{
  return os << "GdkRectangle("
            << r.x << ", "
            << r.y << ", "
            << r.width << ", "
            << r.height << ")";
}
