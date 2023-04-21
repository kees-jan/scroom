#include <gtest/gtest.h>

#include "single-context.hh"

using namespace Scroom::Utils;

class SingleContextTests : public ::testing::Test
{
public:
  SingleContext context;
};

TEST_F(SingleContextTests, store_and_retrieve) // NOLINT
{
  EXPECT_THROW((void)context.get("int"), Context::name_not_found);
  EXPECT_FALSE(context.try_get("int").has_value());

  context.set("int", 3);
  EXPECT_EQ(std::any_cast<int>(context.get("int")), 3);
  EXPECT_EQ(std::any_cast<int>(context.try_get("int")), 3);
}

TEST_F(SingleContextTests, no_duplicate_stores) // NOLINT
{
  context.set("int", 3);
  EXPECT_THROW(context.set("int", 3), Context::name_exists);
}
