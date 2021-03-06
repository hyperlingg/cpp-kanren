#define BOOST_TEST_MODULE cpp - kanren - test
#include "../include/cpp-kanren.h"

#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>
variable x = makeVar("x");
variable y = makeVar("y");
variable z = makeVar("z");
variable x2 = makeVar("x");
variable w = makeVar("w");
variable v = makeVar("v");

constant a = makeConst("a");

value_list ls = {x, y, a};
value_list ls2 = {x, makeConst("e"), z};

association assoc1 = {x, y};
association assoc2 = {y, z};
association assoc3 = {z, ls};

association assoc4 = {z, a};
association assoc5 = {x, w};
association assoc6 = {y, z};

substitution sub1 = {assoc1, assoc2};
substitution sub2 = {assoc4, assoc5, assoc6};
substitution sub3 = {{x, y}, {v, x}, {w, x}};

substitution sub4 = {{x, makeConst("b")}, {z, y}, {w, ls2}};

BOOST_AUTO_TEST_SUITE(variable_equality)

BOOST_AUTO_TEST_CASE(unequal_var) { BOOST_CHECK(x != y); }

BOOST_AUTO_TEST_CASE(equal_var) { BOOST_CHECK(x == x); }

BOOST_AUTO_TEST_CASE(unequal_var_same_name) { BOOST_CHECK(x != x2); }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(string_representations)

BOOST_AUTO_TEST_CASE(variable_string) {
  auto res = getStringValue(x);
  BOOST_CHECK(res.has_value() && res.value() == "x");
}

BOOST_AUTO_TEST_CASE(constant_string) {
  auto res = getStringValue(a);
  BOOST_CHECK(res.has_value() && res.value() == "a");
}

BOOST_AUTO_TEST_CASE(value_list_string) {
  auto res = getStringValue(ls);
  BOOST_CHECK(res.has_value() && res.value() == "( x, y, a )");
}

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

BOOST_AUTO_TEST_SUITE(walk_function_test)

BOOST_AUTO_TEST_CASE(walk_z_frame13) {
  auto walkRes = walk(z, sub2);
  if (holds_alternative<atom>(walkRes)) {
    BOOST_CHECK(get<atom>(walkRes) == a);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_CASE(walk_y_frame14) {
  auto walkRes = walk(y, sub2);
  if (holds_alternative<atom>(walkRes)) {
    BOOST_CHECK(get<atom>(walkRes) == a);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_CASE(walk_x_frame15) {
  auto walkRes = walk(x, sub2);
  if (holds_alternative<atom>(walkRes)) {
    BOOST_CHECK(get<atom>(walkRes) == w);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_CASE(walk_x_frame16) {
  auto walkRes = walk(x, sub3);
  if (holds_alternative<atom>(walkRes)) {
    BOOST_CHECK(get<atom>(walkRes) == y);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_CASE(walk_v_frame16) {
  auto walkRes = walk(x, sub3);
  if (holds_alternative<atom>(walkRes)) {
    BOOST_CHECK(get<atom>(walkRes) == y);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_CASE(walk_w_frame17) {
  auto walkRes = walk(w, sub4);
  if (holds_alternative<value_list>(walkRes)) {
    BOOST_CHECK(get<value_list>(walkRes) == ls2);
  } else {
    BOOST_CHECK(false);
  }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(occurs_function_test)

BOOST_AUTO_TEST_CASE(occurs_true_basic) { BOOST_CHECK(occurs(x, x, {})); }

BOOST_AUTO_TEST_CASE(occurs_true_recurse) {
  value_list ls = {y};
  BOOST_CHECK(occurs(x, ls, {{y, x}}));
}

BOOST_AUTO_TEST_CASE(occurs_false) { BOOST_CHECK(!occurs(z, a, sub3)); }

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(ext_s_function_test)

BOOST_AUTO_TEST_CASE(ext_s_empty) {
  value_list ls = {x};
  BOOST_CHECK(!ext_s(x, ls, {}));
}

BOOST_AUTO_TEST_CASE(ext_s_nempty) {
  value_list ls = {y};
  BOOST_CHECK(!ext_s(x, ls, {{y, x}}));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(unification)

BOOST_AUTO_TEST_CASE(eqv_consts_empty_s) {
  auto goal = eqv(makeConst("a"), makeConst("b"));
  auto resStream = goal(empty_s);  // this is a singleton stream
  substitution singleton = resStream.getValue().value;

  // should be the same as singleton
  auto uRes = u_goal(empty_s)().getValue().value;

  BOOST_CHECK((empty_s == singleton) && (uRes == singleton));
}

BOOST_AUTO_TEST_CASE(eqv_vars_empty_s) {
  auto goal = eqv(x, y);
  auto resStream = goal(empty_s);

  substitution singleton = {{x, y}};

  auto resSubSize = resStream.getValue().value.size();
  BOOST_CHECK(resSubSize == 1);  // why does this fail? size is 0... TODO
}

BOOST_AUTO_TEST_CASE(disj_eqv) {
  auto goal = disj(eqv(makeConst("olive"), y), eqv(makeConst("oil"), y)); // frame 53
  auto resStream = goal(empty_s);

  auto resSubSize = resStream.getValue().value.size();
  BOOST_CHECK(resSubSize == 2);  // why does this fail? size is 0... TODO
}

BOOST_AUTO_TEST_SUITE_END()
