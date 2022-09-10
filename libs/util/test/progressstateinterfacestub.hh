/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <scroom/progressinterfacehelpers.hh>

class ProgressStateInterfaceStub : public Scroom::Utils::ProgressStateInterface
{
public:
  using Ptr = std::shared_ptr<ProgressStateInterfaceStub>;

public:
  State  state{IDLE};
  double progress{0.0};

public:
  static Ptr create();

private:
  ProgressStateInterfaceStub() = default;

public:
  // ProgressStateInterface
  void setProgress(State s, double d) override;
};
