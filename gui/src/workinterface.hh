/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2021 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/interface.hh>

class WorkInterface : private Interface
{
public:
  virtual bool doWork() = 0;
};
