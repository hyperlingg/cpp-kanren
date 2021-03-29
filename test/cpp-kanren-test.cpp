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

//###########################

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

//###########################

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

//###########################

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

//###########################

BOOST_AUTO_TEST_SUITE(occurs_function_test)

BOOST_AUTO_TEST_CASE(occurs_true_basic) { BOOST_CHECK(occurs(x, x, {})); }

BOOST_AUTO_TEST_CASE(occurs_true_recurse) {
  value_list ls = {y};
  BOOST_CHECK(occurs(x, ls, {{y, x}}));
}

BOOST_AUTO_TEST_CASE(occurs_false) { BOOST_CHECK(!occurs(z, a, sub3)); }

BOOST_AUTO_TEST_SUITE_END()

//###########################

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

//###########################

BOOST_AUTO_TEST_SUITE(unification_eqv_disj)

BOOST_AUTO_TEST_CASE(eqv_consts_empty_s) {
  auto goal = eqv(makeConst("a"), makeConst("b"));
  auto resStream = goal(empty_s);  // this is a singleton stream
  resStream.next();                // NOTE this is important...

  substitution singleton = resStream.getValue().value;

  // u_goal should be the same as singleton
  auto uRes = u_goal()(empty_s).getValue().value;

  BOOST_CHECK((empty_s == singleton) && (uRes == empty_s));
}

// frame 10.51
BOOST_AUTO_TEST_CASE(eqv_vars_empty_s) {
  auto goal = eqv(x, y);
  auto resStream = goal(empty_s);
  resStream.next();  // NOTE this is important...
  substitution singleton = {{x, y}};

  auto resSubTag = resStream.getValue().tag;
  auto resSubValue = resStream.getValue().value;
  std::cout << "size: " << resSubValue.size() << std::endl;

  auto sndValueAtom = get<atom>(resSubValue.front().second);

  BOOST_CHECK((resSubTag == 1) && (resSubValue.size() == 1) &&
              (resSubValue.front().first == x) &&
              (sndValueAtom->tag == atomValue::VAR) && sndValueAtom == y);
}

// frame 10.53
BOOST_AUTO_TEST_CASE(disj_eqv) {
  auto oilConst = makeConst("oil");
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, y), eqv(oilConst, y));
  auto resStream = goal(empty_s);
  resStream.next();  // NOTE this is important...

  auto resSubSize = resStream.getValue().value.size();
  atom resSubFstFst = resStream.getValue().value.front().first;
  constant resSubFstSnd =
      get<constant>(resStream.getValue().value.front().second);

  resStream.next();  // NOTE this is important...
  atom resSubSndFst = resStream.getValue().value.front().first;

  // and now checking if the "oil" occurs as well...hm, the "olive" does! maybe
  // its just reverse ordered -> yeah that's it.
  constant sndSubSndSnd =
      get<constant>(resStream.getValue().value.front().second);

  BOOST_CHECK(resSubSize == 1 && resSubFstFst == y && resSubSndFst == y &&
              sndSubSndSnd == oliveConst && resSubFstSnd == oilConst);
}

// frame 10.63
BOOST_AUTO_TEST_CASE(disj_neverO) {
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, x), never_o_goal());
  auto resStream = goal(empty_s);
  resStream.next();  // NOTE this is important...

  atom resSubFstFst = resStream.getValue().value.front().first;
  constant resSubFstSnd =
      get<constant>(resStream.getValue().value.front().second);

  BOOST_CHECK(resSubFstFst == x && resSubFstSnd == oliveConst);
}

// frames 10.64 and 10.66
BOOST_AUTO_TEST_CASE(disj_neverO_switched) {
  auto oliveConst = makeConst("olive");

  auto goal = disj(never_o_goal(), eqv(oliveConst, x));
  auto resStream = goal(empty_s);
  resStream.next();  // getting the first stream element (a suspension)
  auto resSubFstTag = resStream.getValue().tag;
  resStream.next();  // forcing the "suspension"
  atom resSubSndFst = resStream.getValue().value.front().first;
  constant resSubSndSnd =
      get<constant>(resStream.getValue().value.front().second);

  BOOST_CHECK(resSubFstTag == stream_elem::SUSPEND && resSubSndFst == x &&
              resSubSndSnd == oliveConst);
}

BOOST_AUTO_TEST_SUITE_END()

//###########################

BOOST_AUTO_TEST_SUITE(take_inf_alwaysO)

// frame 10.77
BOOST_AUTO_TEST_CASE(take_3) {
  Stream<stream_elem> takeStream = always_o()(empty_s);
  auto take_3_res = take_inf(3, takeStream);

  // while (take_3_res.next()) {
  //   auto fstSubEmpty = take_3_res.getValue().value.empty();
  //   auto tag = take_3_res.getValue().tag;
  //   std::cout << "take_3 empty? " << fstSubEmpty
  //             << " tag?: " << tag << std::endl;
  // }

  take_3_res.next();  // -> true
  auto fstSubEmpty = take_3_res.getValue().value.empty();

  take_3_res.next();  // -> true
  auto sndSubEmpty = take_3_res.getValue().value.empty();

  take_3_res.next();  // -> true
  auto thrdSubEmpty = take_3_res.getValue().value.empty();

  // // check that all three substitutions are empty and that there is no 4th
  // one
  BOOST_CHECK(fstSubEmpty && sndSubEmpty && thrdSubEmpty && !take_3_res.next());
}

// frame 10.79
BOOST_AUTO_TEST_CASE(found2not5) {
  auto oilConst = makeConst("oil");
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, y), eqv(oilConst, y));
  auto resStream = goal(empty_s);

  auto take_5_stream = take_inf(5, resStream);

  int streamLength;
  while (take_5_stream.next()) {
    streamLength++;
    // std::cout << "data found2not5 fst"
    //           << take_5_stream.getValue().value.front().first->data
    //           << std::endl;

    // // snd elem of the front of substitution
    // auto snd = take_5_stream.getValue().value.front().second;
    // if (holds_alternative<atom>(snd)) {
    //   auto atm = get<atom>(snd);
    //   std::cout << "atm->data " << atm->data << std::endl;
    // }

    // // size of substitution
    // std::cout << "size :" << take_5_stream.getValue().value.size() << std::endl;
  }

  // std::cout << "streamlength: " << streamLength << std::endl;
  BOOST_CHECK(streamLength == 2);  // yeah this stream really has 4 (or 5?) elements instead
                      // of 2...why?
}

BOOST_AUTO_TEST_SUITE_END()