/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2023 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <thread>

namespace Scroom::GtkTestHelpers
{
  class GtkMainLoop
  {
  private:
    std::thread gtk_thread;

  public:
    GtkMainLoop();
    GtkMainLoop(const GtkMainLoop&)            = delete;
    GtkMainLoop& operator=(const GtkMainLoop&) = delete;
    GtkMainLoop(GtkMainLoop&&)                 = delete;
    GtkMainLoop& operator=(GtkMainLoop&&)      = delete;
    ~GtkMainLoop();

  private:
    static void run_gtk();
  };

} // namespace Scroom::GtkTestHelpers
