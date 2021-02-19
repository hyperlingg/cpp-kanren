#define BOOST_TEST_MODULE cpp-kanren-test
#include <boost/test/unit_test.hpp>
#include <boost/test/tools/output_test_stream.hpp>

#include "../include/cpp-kanren.h"

variable x = make_shared<string>("x");
variable y = make_shared<string>("y");
variable z = make_shared<string>("z");
variable x2 = make_shared<string>("x");

association assoc1 = {x,y};
association assoc2 = {y,z};
// association assoc3 = {z,{"a",x}}; // TODO rework value defn

substitution sub1 = {assoc1,assoc2};


BOOST_AUTO_TEST_SUITE(variable_equality)

BOOST_AUTO_TEST_CASE(unequal_var)
{
    BOOST_CHECK(x!=y);
}


BOOST_AUTO_TEST_CASE(equal_var)
{
    BOOST_CHECK(x==x);
}

BOOST_AUTO_TEST_CASE(unequal_var_same_name)
{
    BOOST_CHECK(x!=x2);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(assv_function_test)

BOOST_AUTO_TEST_CASE(assv_has_value_x)
{
    optional<association> res = assv(x,sub1);
    BOOST_CHECK(res.has_value() && res.value().first == x);
}

BOOST_AUTO_TEST_CASE(assv_has_value_y)
{
    optional<association> res = assv(y,sub1);
    BOOST_CHECK(res.has_value() && res.value().first == y);
}

BOOST_AUTO_TEST_CASE(assv_no_value)
{
    optional<association> res = assv(z,sub1);
    BOOST_CHECK(!res.has_value());
}

BOOST_AUTO_TEST_SUITE_END()

