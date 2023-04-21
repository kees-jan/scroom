#include <gtest/gtest.h>

#include "parent-context.hh"
#include "single-context.hh"

using namespace Scroom::Utils;

class ParentContextTests : public ::testing::Test
{
public:
  Context::Ptr first;
  Context::Ptr second;
  ParentContext context;

  ParentContextTests()
    : first(std::make_shared<SingleContext>())
    , second(std::make_shared<SingleContext>())
    , context(first)
  {
    context.add(second);
    second->set("int", 3);
  }
};

TEST_F(ParentContextTests, store_and_retrieve) // NOLINT
{
  EXPECT_THROW((void)context.get("absent"), Context::name_not_found);
  EXPECT_FALSE(context.try_get("absent").has_value());
  EXPECT_EQ(std::any_cast<int>(context.get("int")), 3);

  context.set("int", 4);
  EXPECT_EQ(std::any_cast<int>(context.get("int")), 4);
  EXPECT_EQ(std::any_cast<int>(context.try_get("int")), 4);
  EXPECT_EQ(std::any_cast<int>(first->get("int")), 4);
  EXPECT_EQ(std::any_cast<int>(second->get("int")), 3);
}

TEST_F(ParentContextTests, no_duplicate_stores) // NOLINT
{
  context.set("int", 3);
  EXPECT_THROW(context.set("int", 3), Context::name_exists);
}
