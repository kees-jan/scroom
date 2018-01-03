/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2018 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <boost/shared_ptr.hpp>

/**
 * Interface used for reporting progress information
 */
class ProgressInterface
{
public:
  typedef boost::shared_ptr<ProgressInterface> Ptr;
  typedef boost::weak_ptr<ProgressInterface> WeakPtr;

  virtual ~ProgressInterface() {}
  
  virtual void setIdle()=0;
  virtual void setWaiting(double progress=0.0)=0;
  virtual void setWorking(double progress)=0;
  virtual void setFinished()=0;
};


