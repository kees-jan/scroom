/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2022 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <gtk/gtk.h>

#include <scroom/viewinterface.hh>

class ProgressBarManager : public ProgressInterface
{
public:
  using Ptr = boost::shared_ptr<ProgressBarManager>;

private:
  GtkProgressBar* progressBar;
  bool            isWaiting;

private:
  ProgressBarManager(GtkProgressBar* progressBar);

  void stopWaiting();
  void startWaiting();

public:
  static Ptr create(GtkProgressBar* progressBar = nullptr);

  ~ProgressBarManager() override;
  ProgressBarManager(const ProgressBarManager&)           = delete;
  ProgressBarManager(ProgressBarManager&&)                = delete;
  ProgressBarManager operator=(const ProgressBarManager&) = delete;
  ProgressBarManager operator=(ProgressBarManager&&)      = delete;


  void setProgressBar(GtkProgressBar* progressBar);

  // ProgressInterface ///////////////////////////////////////////////////

  void setIdle() override;
  void setWaiting(double progress = 0.0) override;
  void setWorking(double progress) override;
  void setFinished() override;
};
