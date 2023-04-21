#include <boost/test/unit_test.hpp>

#include "single-context.hh"

using namespace Scroom::Utils;

struct ContextFixture
{
  Context::Ptr         context;
  std::shared_ptr<int> value;
  std::weak_ptr<int>   weakValue;

  ContextFixture()
    : context(std::make_shared<SingleContext>())
    , value(std::make_shared<int>(3))
    , weakValue(value)
  {
  }
};

struct Dummy
{
  using Ptr = std::shared_ptr<Dummy>;

  static Ptr Create() { return std::make_shared<Dummy>(); }
};

BOOST_FIXTURE_TEST_SUITE(context_tests, ContextFixture)

BOOST_AUTO_TEST_CASE(store_and_retrieve_values)
{
  const std::string name        = "name";
  const int         an_int      = 5;
  const int         another_int = 4;

  set(context, an_int);
  set(context, name, another_int);
  BOOST_TEST(get<int>(context) == an_int);
  BOOST_TEST(get<int>(context, name) == another_int);
}

BOOST_AUTO_TEST_CASE(store_and_retrieve_shared_pointers)
{
  const std::string name = "name";
  auto              p1   = Dummy::Create();
  auto              p2   = Dummy::Create();
  BOOST_TEST(p1 != p2);

  set(context, p1);
  set(context, name, p2);

  BOOST_TEST(get<Dummy::Ptr>(context) == p1);
  BOOST_TEST(get<Dummy::Ptr>(context, name) == p2);
}

BOOST_AUTO_TEST_CASE(store_and_retrieve_shared_pointers_from_labdas)
{
  const std::string name = "name";
  auto              p1   = Dummy::Create();
  auto              p2   = Dummy::Create();
  BOOST_TEST(p1 != p2);

  setFactory(context, [p1] { return p1; });
  setFactory(context, name, [p2] { return p2; });

  BOOST_TEST(get<Dummy::Ptr>(context) == p1);
  BOOST_TEST(get<Dummy::Ptr>(context, name) == p2);
}

BOOST_AUTO_TEST_CASE(use_defaults_when_values_are_not_available)
{
  const std::string name = "name";
  auto              p1   = Dummy::Create();
  auto              p2   = Dummy::Create();
  BOOST_TEST(p1 != p2);

  BOOST_TEST(get_or<Dummy::Ptr>(context, [p1] { return p1; }) == p1);
  BOOST_TEST(get_or<Dummy::Ptr>(context, name, [p2] { return p2; }) == p2);
  BOOST_TEST(get_or<Dummy::Ptr>(context, p1) == p1);
  BOOST_TEST(get_or<Dummy::Ptr>(context, name, p2) == p2);
}

BOOST_AUTO_TEST_SUITE_END()
