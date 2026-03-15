/*
 * Scroom - Generic viewer for 2D data
 * Copyright (C) 2009-2026 Kees-Jan Dijkzeul
 *
 * SPDX-License-Identifier: LGPL-2.1
 */

#include <sstream>

#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/spdlog.h>

#include <gtest/gtest.h>

#include <scroom/logger.hh>

using Scroom::Logger;
using Scroom::LoggerContainer;

//////////////////////////////////////////////////////////////

class LoggerTests : public ::testing::Test
{
public:
  std::ostringstream   oss;
  LoggerContainer::Ptr loggerContainer;
  Logger               logger;

  LoggerTests()
    : loggerContainer(std::make_shared<LoggerContainer>())
    , logger(loggerContainer)
  {
    auto sink      = std::make_shared<spdlog::sinks::ostream_sink_mt>(oss);
    auto spdlogger = std::make_shared<spdlog::logger>("test", sink);
    spdlogger->set_pattern("%v");
    loggerContainer->set(spdlogger);
    logger->set_level(spdlog::level::debug);
  }
};

//////////////////////////////////////////////////////////////

const std::string testMessage = "hello from logger";

//////////////////////////////////////////////////////////////

TEST_F(LoggerTests, can_log_via_deref) // NOLINT
{
  logger->info(testMessage);

  EXPECT_EQ(testMessage + "\n", oss.str());
}

TEST(LoggerDeathTest, default_logger_is_cleared) // NOLINT
{
  EXPECT_DEATH(spdlog::info(testMessage), "");
}
