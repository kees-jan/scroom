/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <functional>

#include <scroom/layer-interface.hh>
#include <scroom/threadpool.hh>
#include <scroom/tiled-bitmap-data.hh>

void populateLayer(
  LayerInterface::Ptr layer,
  SourcePresentation::Ptr source,
  const ThreadPool::WeakQueue::Ptr& queue,
  std::function<void()> onFinished
);
