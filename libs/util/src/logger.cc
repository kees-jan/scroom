/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <mutex>

#include <spdlog/sinks/stdout_color_sinks.h>

#include <scroom/logger.hh>

namespace Scroom
{
  LoggerContainer::LoggerContainer()
    : logger(std::make_shared<spdlog::logger>("scroom", std::make_shared<spdlog::sinks::stdout_color_sink_mt>()))
  {
  }

  LoggerContainer::Ptr LoggerContainer::instance()
  {
    static std::mutex                     mut;
    static std::weak_ptr<LoggerContainer> instance;
    std::lock_guard const                 lock(mut);
    Ptr                                   result = instance.lock();
    if(!result)
    {
      result   = Ptr(new LoggerContainer());
      instance = result;
    }
    return result;
  }

  std::shared_ptr<spdlog::logger> LoggerContainer::get() const
  {
    std::lock_guard const lock(mut);
    return logger;
  }

  void LoggerContainer::set(std::shared_ptr<spdlog::logger> logger_)
  {
    std::lock_guard const lock(mut);
    logger = std::move(logger_);
  }

  Logger::Logger()
    : Logger(LoggerContainer::instance())
  {
  }

  Logger::Logger(LoggerContainer::Ptr container_)
    : container(std::move(container_))
  {
  }

  std::shared_ptr<spdlog::logger> Logger::operator->() const { return container->get(); }

} // namespace Scroom
