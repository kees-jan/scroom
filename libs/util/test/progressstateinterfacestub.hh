/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2020 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/progressinterfacehelpers.hh>

class ProgressStateInterfaceStub : public Scroom::Utils::ProgressStateInterface
{
public:
  typedef boost::shared_ptr<ProgressStateInterfaceStub> Ptr;

public:
  State  state;
  double progress;

public:
  static Ptr create();
  virtual ~ProgressStateInterfaceStub() {}

private:
  ProgressStateInterfaceStub();

public:
  // ProgressStateInterface
  virtual void setProgress(State s, double d);
};
