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

#ifndef PROGRESSINTERFACE_HH
#define PROGRESSINTERFACE_HH

#include <boost/shared_ptr.hpp>

/**
 * Interface used for reporting progress information
 */
class ProgressInterface
{
public:
  typedef boost::shared_ptr<ProgressInterface> Ptr;
  typedef boost::weak_ptr<ProgressInterface> WeakPtr;

  typedef enum
    {
      IDLE,
      WAITING,
      WORKING,
      FINISHED
    } State;

  virtual ~ProgressInterface() {}
  
  virtual void setState(State s)=0;
  virtual void setProgress(double d)=0;
  virtual void setProgress(int done, int total)=0;
};


// /**
//  * Interface used for reporting progress information
//  */
// class ProgressInterface
// {
// public:
//   typedef boost::shared_ptr<ProgressInterface> Ptr;
//   typedef boost::weak_ptr<ProgressInterface> WeakPtr;
// 
//   virtual ~ProgressInterface() {}
//   
//   virtual void setIdle()=0;
//   virtual void setWaiting(double progress=0.0)=0;
//   virtual void setWorking(double progress)=0;
//   virtual void setWorking(int done, int total)=0;
//   virtual void setFinished()=0;
// };




#endif
