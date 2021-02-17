#define BOOST_TEST_MODULE cpp-kanren-test
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "../include/cpp-kanren.h"

BOOST_AUTO_TEST_SUITE(TestTestSuite)

BOOST_AUTO_TEST_CASE(TestTestCase)
{
    BOOST_CHECK(true);
}

BOOST_AUTO_TEST_SUITE_END()