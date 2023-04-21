/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

class Interface
{
public:
  Interface()                             = default;
  Interface(const Interface&)             = delete;
  Interface& operator=(const Interface&)  = delete;
  Interface(const Interface&&)            = delete;
  Interface& operator=(const Interface&&) = delete;
  virtual ~Interface()                    = default;
};