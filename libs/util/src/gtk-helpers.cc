/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2017 Kees-Jan Dijkzeul
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

      Wrapper::Wrapper(const boost::function<bool()>& f)
      :f(f)
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

    Wrapper::Wrapper(const boost::function<bool()>& f)
        : f(&Detail::gtkWrapper), data(Detail::Wrapper::create(f))
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
