/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#pragma once

#include <memory>

#include <spdlog/spdlog.h>

namespace Scroom
{
  namespace Detail
  {
    // Including this header clears the spdlog default logger, causing any code
    // that still uses spdlog free functions (spdlog::info() etc.) to crash.
    // Use Scroom::Logger instead.
    struct ClearDefaultLogger
    {
      ClearDefaultLogger() { spdlog::set_default_logger(nullptr); }
    };

    inline const ClearDefaultLogger clearDefaultLogger;

  } // namespace Detail

  class LoggerContainer
  {
  public:
    using Ptr = std::shared_ptr<LoggerContainer>;

    static Ptr instance();

    LoggerContainer();

    std::shared_ptr<spdlog::logger> get() const;
    void                            set(std::shared_ptr<spdlog::logger> logger_);

  private:
    mutable std::mutex              mut;
    std::shared_ptr<spdlog::logger> logger;
  };

  class Logger
  {
  public:
    Logger();
    explicit Logger(LoggerContainer::Ptr container_);

    std::shared_ptr<spdlog::logger> operator->() const;

  private:
    LoggerContainer::Ptr container;
  };

} // namespace Scroom
