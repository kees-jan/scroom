#include <boost/test/unit_test.hpp>

#include "parent-context.hh"
#include "single-context.hh"

using namespace Scroom::Utils;

struct ParentContextFixture
{
  Context::Ptr  first;
  Context::Ptr  second;
  ParentContext context;

  ParentContextFixture()
    : first(std::make_shared<SingleContext>())
    , second(std::make_shared<SingleContext>())
    , context(first)
  {
    context.add(second);
    second->set("int", 3);
  }
};

BOOST_FIXTURE_TEST_SUITE(recursive_context_tests, ParentContextFixture)

BOOST_AUTO_TEST_CASE(store_and_retrieve)
{
  BOOST_CHECK_THROW((void)context.get("absent"), Context::name_not_found);
  BOOST_CHECK(!context.try_get("absent").has_value());
  BOOST_TEST(std::any_cast<int>(context.get("int")) == 3);

  context.set("int", 4);
  BOOST_TEST(std::any_cast<int>(context.get("int")) == 4);
  BOOST_TEST(std::any_cast<int>(context.try_get("int")) == 4);
  BOOST_TEST(std::any_cast<int>(first->get("int")) == 4);
  BOOST_TEST(std::any_cast<int>(second->get("int")) == 3);
}

BOOST_AUTO_TEST_CASE(no_duplicate_stores)
{
  context.set("int", 3);
  BOOST_CHECK_THROW(context.set("int", 3), Context::name_exists);
}

BOOST_AUTO_TEST_SUITE_END()
