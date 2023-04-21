#include <gtest/gtest.h>

#include "single-context.hh"

using namespace Scroom::Utils;

struct Dummy
{
  using Ptr = std::shared_ptr<Dummy>;

  static Ptr Create() { return std::make_shared<Dummy>(); }
};

class ContextTests : public ::testing::Test
{
public:
  Context::Ptr context;
  std::shared_ptr<int> value;
  std::weak_ptr<int> weakValue;

  ContextTests()
    : context(std::make_shared<SingleContext>())
    , value(std::make_shared<int>(3))
    , weakValue(value)
  {
  }
};

TEST_F(ContextTests, store_and_retrieve_values) // NOLINT
{
  const std::string name = "name";
  const int an_int = 5;
  const int another_int = 4;

  set(context, an_int);
  set(context, name, another_int);
  EXPECT_EQ(get<int>(context), an_int);
  EXPECT_EQ(get<int>(context, name), another_int);
}

TEST_F(ContextTests, store_and_retrieve_shared_pointers) // NOLINT
{
  const std::string name = "name";
  auto p1 = Dummy::Create();
  auto p2 = Dummy::Create();
  EXPECT_NE(p1, p2);

  set(context, p1);
  set(context, name, p2);

  EXPECT_EQ(get<Dummy::Ptr>(context), p1);
  EXPECT_EQ(get<Dummy::Ptr>(context, name), p2);
}

TEST_F(ContextTests, store_and_retrieve_shared_pointers_from_lambdas) // NOLINT
{
  const std::string name = "name";
  auto p1 = Dummy::Create();
  auto p2 = Dummy::Create();
  EXPECT_NE(p1, p2);

  setFactory(context, [p1] { return p1; });
  setFactory(context, name, [p2] { return p2; });

  EXPECT_EQ(get<Dummy::Ptr>(context), p1);
  EXPECT_EQ(get<Dummy::Ptr>(context, name), p2);
}

TEST_F(ContextTests, use_defaults_when_values_are_not_available) // NOLINT
{
  const std::string name = "name";
  auto p1 = Dummy::Create();
  auto p2 = Dummy::Create();
  EXPECT_NE(p1, p2);

  EXPECT_EQ(get_or<Dummy::Ptr>(context, [p1] { return p1; }), p1);
  EXPECT_EQ(get_or<Dummy::Ptr>(context, name, [p2] { return p2; }), p2);
  EXPECT_EQ(get_or<Dummy::Ptr>(context, p1), p1);
  EXPECT_EQ(get_or<Dummy::Ptr>(context, name, p2), p2);
}

TEST_F(ContextTests, try_get_returns_nullopt_when_absent) // NOLINT
{
  const std::string name = "name";

  EXPECT_EQ(try_get<Dummy::Ptr>(context), std::nullopt);
  EXPECT_EQ(try_get<Dummy::Ptr>(context, name), std::nullopt);
}

TEST_F(ContextTests, try_get_returns_value_when_present) // NOLINT
{
  const std::string name = "name";
  auto p1 = Dummy::Create();
  auto p2 = Dummy::Create();
  EXPECT_NE(p1, p2);

  set(context, p1);
  set(context, name, p2);

  EXPECT_EQ(try_get<Dummy::Ptr>(context), p1);
  EXPECT_EQ(try_get<Dummy::Ptr>(context, name), p2);
}

TEST_F(ContextTests, try_get_returns_value_from_factory_when_present) // NOLINT
{
  const std::string name = "name";
  auto p1 = Dummy::Create();
  auto p2 = Dummy::Create();
  EXPECT_NE(p1, p2);

  setFactory(context, [p1] { return p1; });
  setFactory(context, name, [p2] { return p2; });

  EXPECT_EQ(try_get<Dummy::Ptr>(context), p1);
  EXPECT_EQ(try_get<Dummy::Ptr>(context, name), p2);
}
