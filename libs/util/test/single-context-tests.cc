#include <boost/noncopyable.hpp>
#include <boost/test/unit_test.hpp>

#include "single-context.hh"

using namespace Scroom::Utils;


struct SingleContextFixture
{
  SingleContext context;
};

BOOST_FIXTURE_TEST_SUITE(single_context_tests, SingleContextFixture)

BOOST_AUTO_TEST_CASE(store_and_retrieve)
{
  BOOST_CHECK_THROW((void)context.get("int"), Context::name_not_found);
  BOOST_CHECK(!context.try_get("int").has_value());

  context.set("int", 3);
  BOOST_TEST(std::any_cast<int>(context.get("int")) == 3);
  BOOST_TEST(std::any_cast<int>(context.try_get("int")) == 3);
}

BOOST_AUTO_TEST_CASE(no_duplicate_stores)
{
  context.set("int", 3);
  BOOST_CHECK_THROW(context.set("int", 3), Context::name_exists);
}

BOOST_AUTO_TEST_SUITE_END()
