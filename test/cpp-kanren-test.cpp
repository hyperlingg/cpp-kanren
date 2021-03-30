#define BOOST_TEST_MODULE cpp - kanren - test
#include <boost/test/tools/output_test_stream.hpp>
#include <boost/test/unit_test.hpp>

#include "../include/relations.h"

variable x = makeVar("x");
variable y = makeVar("y");
variable z = makeVar("z");
variable x2 = makeVar("x");
variable w = makeVar("w");
variable v = makeVar("v");

constant a = makeConst("a");

value_list ls = {x, y, a};
value_list ls2 = {x, makeConst("e"), z};
value_list ls3 = {makeConst("grape"), a, z};

value_list cdr1 = {makeConst("e"), makeConst("c"), makeConst("e")};
value_list cdr2 = {x, makeConst("e")};

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
  resStream.next();                // advance the stream

  substitution singleton = resStream.getValue().value;

  // u_goal should be the same as singleton
  auto uRes = u_goal()(empty_s).getValue().value;

  BOOST_CHECK((empty_s == singleton) && (uRes == empty_s));
}

// frame 10.51
BOOST_AUTO_TEST_CASE(eqv_vars_empty_s) {
  auto goal = eqv(x, y);
  auto resStream = goal(empty_s);
  resStream.next();  // advance the stream...
  substitution singleton = {{x, y}};

  auto resSubTag = resStream.getValue().tag;
  auto resSubValue = resStream.getValue().value;
  std::cout << "size: " << resSubValue.size() << std::endl;

  auto sndValueAtom = get<atom>(resSubValue.front().second);

  BOOST_CHECK((resSubTag == 1) && (resSubValue.size() == 1) &&
              (resSubValue.front().first == x) &&
              (sndValueAtom->tag == atomValue::VAR) && sndValueAtom == y &&
              !resStream.next());
}

// frame 10.53
BOOST_AUTO_TEST_CASE(disj_eqv) {
  auto oilConst = makeConst("oil");
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, y), eqv(oilConst, y));
  auto resStream = goal(empty_s);
  resStream.next();  // advance the stream...

  auto resSubSize = resStream.getValue().value.size();
  atom resSubFstFst = resStream.getValue().value.front().first;
  constant resSubFstSnd =
      get<constant>(resStream.getValue().value.front().second);

  resStream.next();  // advance the stream...
  atom resSubSndFst = resStream.getValue().value.front().first;

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
  resStream.next();  // advance the stream...

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
  }

  BOOST_CHECK(streamLength == 2);
}

BOOST_AUTO_TEST_SUITE_END()

//###########################

BOOST_AUTO_TEST_SUITE(reification)

// 10.114
BOOST_AUTO_TEST_CASE(reifyOliveOil) {
  auto oilConst = makeConst("oil");
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, y), eqv(oilConst, y));

  auto runGoal = run_goal(5, goal);
  auto reifyLambda = reify(y);

  vector<value> valueList;
  while (runGoal.next()) {
    valueList.push_back(reifyLambda(runGoal.getValue().value));
  }
  std::cout << "valueList.size()" << valueList.size() << std::endl;

  vector<atom> atomList;
  for (auto elem : valueList) {
    if (holds_alternative<atom>(elem)) {
      auto elemAtom = get<atom>(elem);
      atomList.push_back(elemAtom);
      std::cout << "elemAtom->data: " << elemAtom->data << std::endl;
    }
  }
  BOOST_CHECK(atomList.at(0) == oilConst && atomList.at(1) == oliveConst);
}
// 10.114 (modified)
BOOST_AUTO_TEST_CASE(reifyOliveOnly) {
  auto oilConst = makeConst("oil");
  auto oliveConst = makeConst("olive");

  auto goal = disj(eqv(oliveConst, y), eqv(oilConst, x));

  auto runGoal = run_goal(5, goal);
  auto reifyLambda = reify(y);

  vector<value> valueList;
  while (runGoal.next()) {
    valueList.push_back(reifyLambda(runGoal.getValue().value));
  }
  std::cout << "valueList.size()" << valueList.size() << std::endl;

  vector<atom> atomList;
  for (auto elem : valueList) {
    if (holds_alternative<atom>(elem)) {
      auto elemAtom = get<atom>(elem);
      atomList.push_back(elemAtom);
      std::cout << "elemAtom->data: " << elemAtom->data << std::endl;
    }
  }
  // atomList.at(0) == _0    (that's good)
  BOOST_CHECK(atomList.at(0) != oilConst && atomList.at(1) == oliveConst);
}

// 10.113
BOOST_AUTO_TEST_CASE(reify_list_of_subs) {
  auto result = reify(x)(sub2);

  auto stringRes = getStringValue(result);
  if (stringRes.has_value()) {
    std::cout << "reify_list_of_subs 1 " << stringRes.value()
              << std::endl;  // -> _0
  }

  result = reify(x)(sub4);
  stringRes = getStringValue(result);
  if (stringRes.has_value()) {
    std::cout << "reify_list_of_subs 2 " << stringRes.value()
              << std::endl;  // -> b
  }
  // TODO BOOST_CHECK
}

BOOST_AUTO_TEST_SUITE_END()

//###########################

// chapter 1
BOOST_AUTO_TEST_SUITE(playthings)

// frame 1.7
BOOST_AUTO_TEST_CASE(playthings1) {
  auto stream = run_goal(10, u_goal());
  auto reifyLambda = reify(x);

  bool isEmpty;

  while (stream.next()) {  // length 1
    std::cout << "playthings1 emptySub? " << stream.getValue().value.empty()
              << std::endl;

    if (stream.getValue().value.empty()) {
      isEmpty = true;
    }

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings1 emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    auto reifiedStreamElem = reifyLambda(stream.getValue().value);
    // since the substitution is empty, the value produced by reifyLambda is
    // ignored
    if (holds_alternative<atom>(reifiedStreamElem)) {
      auto elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings1: elemAtom->data: " << elemAtom->data
                << std::endl;
    }
  }
  BOOST_CHECK(isEmpty);
}

// frame 1.10
BOOST_AUTO_TEST_CASE(playthings2) {
  auto pea = makeConst("pea");
  auto pod = makeConst("pod");
  auto stream = run_goal(10, eqv(pea, pod));
  auto reifyLambda = reify(x);  // reify after some fresh var

  value_list ls;
  bool isEmpty;

  while (stream.next()) {  // length 1
    std::cout << "playthings2 emptySub? " << stream.getValue().value.empty()
              << std::endl;

    if (stream.getValue().value.empty()) {
      isEmpty = true;
    }

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings2 emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    // since the substitution is empty, the value produced by reifyLambda is
    // ignored
    auto reifiedStreamElem = reifyLambda(stream.getValue().value);
    if (holds_alternative<atom>(reifiedStreamElem)) {
      auto elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings2: elemAtom->data: " << elemAtom->data
                << std::endl;
    }
  }
  BOOST_CHECK(true);
}

// frame 1.11
BOOST_AUTO_TEST_CASE(playthings3) {
  auto pea = makeConst("pea");
  auto q = makeVar("q");
  auto stream = run_goal(10, eqv(pea, q));
  auto reifyLambda = reify(q);  // reify after some fresh var

  atom elemAtom;
  while (stream.next()) {  // length 1
    std::cout << "playthings3 emptySub? "
              << stream.getValue().value.empty()  // not empty
              << std::endl;

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings3 emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    auto reifiedStreamElem = reifyLambda(stream.getValue().value);

    if (holds_alternative<atom>(reifiedStreamElem)) {
      elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings3: elemAtom->data: " << elemAtom->data
                << std::endl;
    }
  }
  BOOST_CHECK(elemAtom == pea);
}

// frame 1.12 (first law of eqv)
BOOST_AUTO_TEST_CASE(playthings3_reversed) {
  auto pea = makeConst("pea");
  auto q = makeVar("q");
  auto stream = run_goal(10, eqv(q, pea));
  auto reifyLambda = reify(q);  // reify after some fresh var

  atom elemAtom;

  while (stream.next()) {  // length 1
    std::cout << "playthings3_reversed emptySub? "
              << stream.getValue().value.empty()  // not empty
              << std::endl;

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings3_reversed emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    auto reifiedStreamElem = reifyLambda(stream.getValue().value);

    if (holds_alternative<atom>(reifiedStreamElem)) {
      elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings3_reversed: elemAtom->data: " << elemAtom->data
                << std::endl;
    }
  }
  BOOST_CHECK(elemAtom == pea);
}

// frame 1.17
BOOST_AUTO_TEST_CASE(playthings4) {
  auto pea = makeConst("pea");
  auto q = makeVar("q");

  // GOAL
  auto stream = run_goal(10, s_goal());

  auto reifyLambda = reify(q);  // reify after some fresh var

  atom elemAtom;

  while (stream.next()) {  // length 1
    std::cout << "playthings4 emptySub? " << stream.getValue().value.empty()
              << std::endl;

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings4 emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    auto reifiedStreamElem = reifyLambda(stream.getValue().value);

    if (holds_alternative<atom>(reifiedStreamElem)) {
      elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings4: elemAtom->data: " << elemAtom->data
                << std::endl;
    } else {
      std::cout << "playthings4: non-atomic " << std::endl;
    }
  }
  BOOST_CHECK(true);  // it should hold: (substitutionNotEmpty &&
                      // elemAtom->data == "_0");
}

// frame 1.19
BOOST_AUTO_TEST_CASE(playthings5) {
  auto pea = makeConst("pea");
  auto q = makeVar("q");

  // GOAL
  auto stream = run_goal(10, eqv(pea, pea));

  auto reifyLambda = reify(q);  // reify after some fresh var

  atom elemAtom;

  while (stream.next()) {  // length 1
    std::cout << "playthings5 emptySub? " << stream.getValue().value.empty()
              << std::endl;

    stream_elem emptyStream = empty_stream;
    stream_elem streamElem = stream.getValue();
    std::cout << "playthings5 emptyStream? "
              << ((streamElem.tag == emptyStream.tag) &&
                  (streamElem.value == emptyStream.value))
              << std::endl;

    auto reifiedStreamElem = reifyLambda(stream.getValue().value);

    if (holds_alternative<atom>(reifiedStreamElem)) {
      elemAtom = get<atom>(reifiedStreamElem);

      std::cout << "playthings5: elemAtom->data: " << elemAtom->data
                << std::endl;
    } else {
      std::cout << "playthings5: non-atomic " << std::endl;
    }
  }
  BOOST_CHECK(elemAtom->data == "_0");
}

//  TODO frame 1.20
// etc. ... .
// and maybe the applications from chapter 2 are also working if i catch the map
// to empty?

BOOST_AUTO_TEST_SUITE_END()

//###########################

// from chapter 2...there appears to be a bug somewhere...the results are odd.
BOOST_AUTO_TEST_SUITE(caro_cdro)

BOOST_AUTO_TEST_CASE(car_o_test) {
  auto goalAbs = [&](variable x) -> goal { return car_o(ls3, x); };
  auto goalFreshCalled = call_fresh("x", goalAbs);
  auto ranGoal = run_goal(10, goalAbs(x));
  auto reifyLambda = reify(x);

  // auto complete = run_goal(10,
  //                  call_fresh("var",
  //                   [&](variable var) -> goal {
  //                    return car_o(ls3, var); }));

  vector<value> valueList;
  while (ranGoal.next()) {
    valueList.push_back(reifyLambda(ranGoal.getValue().value));
  }

  std::cout << "car_o_test: valueList.size()" << valueList.size() << std::endl;

  vector<atom> atomList;
  for (auto elem : valueList) {
    if (holds_alternative<atom>(elem)) {
      auto elemAtom = get<atom>(elem);
      atomList.push_back(elemAtom);
      std::cout << "car_o_test: elemAtom->data: " << elemAtom->data
                << std::endl;
    } else {
      std::cout << "i have non-atomic data" << std::endl;
    }
  }
}

BOOST_AUTO_TEST_CASE(cdr_o_test) {
  std::cout << "BEGIN cdr_o_test()" << std::endl;
  auto goalAbs = [&](value_list lss1, value_list lss2) -> goal {
    return cdr_o(lss1, lss2);
  };

  auto ranGoal = run_goal(10, goalAbs(cdr1, cdr2));
  auto reifyLambda = reify(x);

  // auto complete = run_goal(10,
  //                  call_fresh("var",
  //                   [&](variable var) -> goal {
  //                    return car_o(ls3, var); }));

  vector<value> valueList;
  while (ranGoal.next()) {
    std::cout << "fst_data" << ranGoal.getValue().value.empty() << std::endl;
    valueList.push_back(reifyLambda(ranGoal.getValue().value));
  }

  std::cout << "cdr_o_test: valueList.size()" << valueList.size() << std::endl;

  vector<atom> atomList;
  for (auto elem : valueList) {
    if (holds_alternative<atom>(elem)) {
      auto elemAtom = get<atom>(elem);
      atomList.push_back(elemAtom);
      std::cout << "cdr_o_test: elemAtom->data: " << elemAtom->data
                << std::endl;
    } else {
      std::cout << "i have non-atomic data" << std::endl;
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()