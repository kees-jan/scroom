#include <scroom/utilities.hh>

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>

using namespace Scroom::Utils;

//////////////////////////////////////////////////////////////

class TestCounted : public Counted<TestCounted>
{
};

//////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE(Counter_Tests)

BOOST_AUTO_TEST_CASE(count)
{
  Counter::Ptr counter = getCounter();
  std::string testCountedName = typeid(TestCounted).name();
  std::map<std::string, unsigned long*> counts = counter->getCounts();
  BOOST_CHECK(!counts.empty());
  BOOST_CHECK(counts.end() != counts.find(testCountedName));
  BOOST_CHECK_EQUAL(0, *counts[testCountedName]);

  {
    TestCounted t;
    counts = counter->getCounts();
    BOOST_CHECK(!counts.empty());
    BOOST_CHECK(counts.end() != counts.find(testCountedName));
    BOOST_CHECK_EQUAL(1, *counts[testCountedName]);
    {
      TestCounted t2;
      counts = counter->getCounts();
      BOOST_CHECK(!counts.empty());
      BOOST_CHECK(counts.end() != counts.find(testCountedName));
      BOOST_CHECK_EQUAL(2, *counts[testCountedName]);
    }
    counts = counter->getCounts();
    BOOST_CHECK(!counts.empty());
    BOOST_CHECK(counts.end() != counts.find(testCountedName));
    BOOST_CHECK_EQUAL(1, *counts[testCountedName]);
  }
  
  counts = counter->getCounts();
  BOOST_CHECK(!counts.empty());
  BOOST_CHECK(counts.end() != counts.find(testCountedName));
  BOOST_CHECK_EQUAL(0, *counts[testCountedName]);
}

BOOST_AUTO_TEST_SUITE_END()
