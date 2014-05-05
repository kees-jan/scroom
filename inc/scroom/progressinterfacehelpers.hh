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

#ifndef PROGRESSINTERFACEHELPERS_HH
#define PROGRESSINTERFACEHELPERS_HH

#include <scroom/progressinterface.hh>

namespace Scroom
{
  namespace Utils
  {
    class ProgressInterfaceFromProgressStateInterface : public ProgressInterface, protected ProgressStateInterface
    {
    public:

      // ProgressInterface ///////////////////////////////////////////////////

      virtual void setIdle();
      virtual void setWaiting(double progress=0.0);
      virtual void setWorking(double progress);
      virtual void setWorking(int done, int total);
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

      // ProgressStateInterface //////////////////////////////////////////////
      virtual void setState(State s);
      virtual void setProgress(double progress);
      virtual void setProgress(int done, int total);

    };
  }
}

#endif
