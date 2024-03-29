/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <scroom/interface.hh>

/**
 * Interface used for reporting progress information
 */
class ProgressInterface : private Interface
{
public:
  using Ptr     = std::shared_ptr<ProgressInterface>;
  using WeakPtr = std::weak_ptr<ProgressInterface>;

  virtual void setIdle()                         = 0;
  virtual void setWaiting(double progress = 0.0) = 0;
  virtual void setWorking(double progress)       = 0;
  virtual void setFinished()                     = 0;
};
