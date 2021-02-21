#define BOOST_TEST_MODULE cpp - kanren - test
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

#include "../include/cpp-kanren.h"
atomValue atmvVal = {atomValue::VAR, "x"};
variable x = make_shared<atomValue>(atmvVal);
// variable x = make_shared<string>("x");
atomValue atmvVal2 = {atomValue::VAR, "y"};
variable y = make_shared<atomValue>(atmvVal2);
atomValue atmvVal3 = {atomValue::VAR, "z"};
variable z = make_shared<atomValue>(atmvVal3);
atomValue atmvVal4 = {atomValue::VAR, "x"};
variable x2 = make_shared<atomValue>(atmvVal4);

atomValue atmvVal5 = {atomValue::CONST, "a"};
constant a = make_shared<atomValue>(atmvVal5);

vector<atom> ls = {x, y, a};
value valueLs = ls;

association assoc1 = {x, y};
association assoc2 = {y, z};
association assoc3 = {z, valueLs};

substitution sub1 = {assoc1, assoc2};

BOOST_AUTO_TEST_SUITE(variable_equality)

BOOST_AUTO_TEST_CASE(unequal_var) { BOOST_CHECK(x != y); }

BOOST_AUTO_TEST_CASE(equal_var) { BOOST_CHECK(x == x); }

BOOST_AUTO_TEST_CASE(unequal_var_same_name) { BOOST_CHECK(x != x2); }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(assv_function_test)

BOOST_AUTO_TEST_CASE(assv_has_value_x) {
  optional<association> res = assv(x, sub1);
  BOOST_CHECK(res.has_value() && res.value().first == x);
}

BOOST_AUTO_TEST_CASE(assv_has_value_y) {
  optional<association> res = assv(y, sub1);
  BOOST_CHECK(res.has_value() && res.value().first == y);
}

BOOST_AUTO_TEST_CASE(assv_no_value) {
  optional<association> res = assv(z, sub1);
  BOOST_CHECK(!res.has_value());
}

BOOST_AUTO_TEST_SUITE_END()
