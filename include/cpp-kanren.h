/*
Header-only implementation of mini-kanren
Author : Jonas Lingg (2021)
*/

#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <variant>
#include <vector>

#include "stream.h"

using namespace std;

// if this works: make it a class member
queue<goal_stream> goalStreamQueue;
queue<value> eqvValueQueue;
queue<goal_stream> ifteQueue;
queue<goal_stream> onceQueue;

#define empty_s ((substitution){})
#define empty_stream \
  { stream_elem::VALUE, empty_s }

// based on frame 11
bool isEmptyS(substitution sub) { return sub.empty(); }

variable makeVar(string name) {
  atomValue atmvVal = {atomValue::VAR, name};
  return make_shared<atomValue>(atmvVal);
}

constant makeConst(string name) {
  atomValue atmvVal = {atomValue::CONST, name};
  return make_shared<atomValue>(atmvVal);
}

// NOTE only depth 1 (non-recursive)
optional<string> getStringValue(value val) {
  if (holds_alternative<atom>(val)) {
    atom atm = get<atom>(val);
    if (atm) {
      return atm->data;
    }
  }

  if (holds_alternative<value_list>(val)) {
    value_list ls = get<value_list>(val);
    string resultString = "( ";
    for (auto iter = ls.begin(); iter != ls.end(); iter++) {
      if (iter != ls.begin()) {
        resultString += ", ";
      }

      resultString += iter->get()->data;
    }
    resultString += " )";
    return resultString;
  }

  return {};
}

// frame 18
optional<association> assv(value val, substitution sub) {
  if (holds_alternative<atom>(val)) {
    atom atm = get<atom>(val);
    if (atm->tag == atomValue::VAR) {
      for (auto it = sub.begin(); it != sub.end(); ++it) {
        if (it->first == atm) {
          return make_optional(*it);
        }
      }
    }
  }
  return {};
}

value walk(value val, substitution sub) {
  if (holds_alternative<atom>(val)) {
    auto assvOpt = assv(val, sub);
    if (assvOpt.has_value()) {
      auto assvValue = assvOpt.value();
      auto snd = assvValue.second;
      return walk(snd, sub);
    }
  }
  return val;
}

value walk_star(value val, substitution sub) {
  auto walked_value = walk(val, sub);

  if (holds_alternative<atom>(walked_value)) {
    auto walked_value_atom = get<atom>(walked_value);

    // debug print
    std::cout << "walk_star(): walked_value_atom->data: "
              << walked_value_atom->data << std::endl;
    // end debug
    return walked_value;
  }

  value_list resList;
  if (holds_alternative<vector<atom>>(walked_value)) {
    auto walked_value_pair = get<vector<atom>>(walked_value);

    for (auto elem : walked_value_pair) {
      value res = walk_star(elem, sub);

      // TODO this test is a workaround that restricts the functionality of the
      // language! walk_star should also be able to append non-atomic values
      if (holds_alternative<atom>(res)) {
        resList.push_back(get<atom>(res));
      }
      // end workaround
    }

    return resList;
  }

  return val;  // TODO verify this return type
}

bool occurs(atom var, value val, substitution sub) {
  // TODO check var for being a var
  auto walkRes = walk(val, sub);

  if (holds_alternative<atom>(walkRes)) {
    auto walkVar = get<atom>(walkRes);
    if (walkVar->tag == atomValue::VAR) {
      return walkVar == var;
    }
  }

  if (holds_alternative<vector<atom>>(walkRes)) {
    auto walkPair = get<vector<atom>>(walkRes);

    for (auto elem : walkPair) {
      bool occ;
      if (elem->tag == atomValue::VAR) {
        occ = occurs(var, elem, sub);
      }

      // TODO is this even necessary?
      // occur check is interested in vars
      if (elem->tag == atomValue::CONST) {
        occ = occurs(var, elem, sub);
      }

      if (occ) {
        return true;
      }
    }
  }

  return false;
}

optional<substitution> ext_s(atom var, value val, substitution sub) {
  // TODO check var for being a var
  std::cout << "enter ext_s" << std::endl;
  if (occurs(var, val, sub)) {
    std::cout << "ext_s : occurs(var, val, sub)" << std::endl;
    return {};
  }

  std::cout << "ext_s : before push_back, sub size: " << sub.size()
            << std::endl;
  sub.push_back({var, val});
  std::cout << "ext_s : after push_back, sub size: " << sub.size() << std::endl;
  return sub;
}

optional<substitution> unify(value u, value v, substitution sub) {
  auto uWalk = walk(u, sub);
  auto vWalk = walk(v, sub);
  std::cout << "enter unify" << std::endl;
  if (uWalk == vWalk) {
    std::cout << "uWalk == vWalk" << std::endl;
    return sub;
  }

  if (holds_alternative<atom>(uWalk)) {
    std::cout << "holds_alternative<atom>(uWalk)" << std::endl;
    auto uAtom = get<atom>(uWalk);
    if (uAtom->tag == atomValue::VAR) {
      std::cout << "uAtom->tag == atomValue::VAR" << std::endl;
      auto ext_sOpt = ext_s(uAtom, v, sub);
      if (ext_sOpt.has_value()) {
        std::cout << "ext_s has value" << std::endl;
        return ext_sOpt.value();
      }
    }
  }

  if (holds_alternative<atom>(vWalk)) {
    std::cout << "holds_alternative<atom>(vWalk)" << std::endl;
    auto vAtom = get<atom>(vWalk);
    if (vAtom->tag == atomValue::VAR) {
      std::cout << "vAtom->tag == atomValue::VAR" << std::endl;
      return ext_s(vAtom, u, sub);
    }
  }

  if (holds_alternative<value_list>(uWalk) &&
      holds_alternative<value_list>(vWalk)) {
    auto vList = get<value_list>(vWalk);
    auto uList = get<value_list>(uWalk);

    for (auto elem : vList) {
      if (!uList.empty()) {
        auto sUnify = unify(uList.front(), elem, sub);
        if (!sUnify.has_value()) {  // 'and' condition
          break;
        }
      } else {
        return {};  // last cond line (else #f)
      }

      uList.erase(uList.begin());  // pop the first element
    }
  }

  std::cout << "before return {}" << std::endl;
  return {};
}

Stream<stream_elem> append_inf(Stream<stream_elem>& s,
                               Stream<stream_elem>& t) noexcept {
  while (s.next()) {
    if (s.getValue().tag ==
        stream_elem::SUSPEND) {  // if suspension -> swap streams
      co_yield s.getValue();     // TODO test this line...
      std::swap(t, s);
    } else {
      co_yield s.getValue();
    }
  }

  while (t.next()) {
    co_yield t.getValue();
  }
}

// NOTE resStream is an empty stream initially (or maybe a suspension?)
Stream<stream_elem> append_map_helper(goal_stream g, Stream<stream_elem>& s,
                                      Stream<stream_elem>& resStream) noexcept {
  while (s.next()) {
    if (s.getValue().tag == stream_elem::VALUE) {
      substitution sub = s.getValue().value;
      auto mapStream = g(sub);
      resStream = append_inf(resStream, mapStream);
    }
  }

  while (resStream.next()) {
    co_yield resStream.getValue();
  }
}

Stream<stream_elem> append_map(goal_stream g, Stream<stream_elem>& s) noexcept {
  Stream<stream_elem> resStream =
      append_map_helper(g, s, s);  // TODO empty stream
  while (resStream.next()) {
    co_yield resStream.getValue();
  }
}

Stream<stream_elem> conj_helper(substitution sub) noexcept {
  // pop g1 and g2 from the argument stack:
  goal_stream g1, g2;
  if (!goalStreamQueue.empty()) {
    g1 = goalStreamQueue.front();
    goalStreamQueue.pop();
    if (!goalStreamQueue.empty()) {
      g2 = goalStreamQueue.front();
      goalStreamQueue.pop();
    } else {
      terminate();
    }
  } else {
    terminate();
  }

  auto appendMapStream = g1(sub);
  Stream<stream_elem> appendStream = append_map(g2, appendMapStream);
  while (appendStream.next()) {
    co_yield appendStream.getValue();
  }
}

goal_stream conj(goal_stream g1, goal_stream g2) noexcept {
  // push g1 and g2 to an argument queue, then return function pointer
  // TODO decide on order of push operation (it's g2 first atm)
  goalStreamQueue.emplace(g2);
  goalStreamQueue.emplace(g1);
  return conj_helper;
}

// TODO can i ensure that disj_helper gets the right arguments?
// if this insurance becomes necessary it will get inelegant, e.g.
// by tagging the function pointer returned by disj as well as the stacked args
// with an ID
Stream<stream_elem> disj_helper(substitution sub) noexcept {
  // pop g1 and g2 from the argument stack:
  goal_stream g1, g2;
  if (!goalStreamQueue.empty()) {
    g1 = goalStreamQueue.front();
    goalStreamQueue.pop();
    if (!goalStreamQueue.empty()) {
      g2 = goalStreamQueue.front();
      goalStreamQueue.pop();
    } else {
      terminate();
    }
  } else {
    terminate();
  }
  // generate the two streams to be appended:
  auto stream1 = g1(sub);
  auto stream2 = g2(sub);

  Stream<stream_elem> appendStream = append_inf(stream1, stream2);
  while (appendStream.next()) {
    co_yield appendStream.getValue();
  }
}

// TODO convert goal_stream to goal by distinguishing variants
goal_stream disj(goal_stream g1, goal_stream g2) noexcept {
  // push g1 and g2 to an argument queue, then return function pointer
  goalStreamQueue.emplace(g1);
  goalStreamQueue.emplace(g2);
  return disj_helper;
}

Stream<stream_elem> take_inf(int n, Stream<stream_elem>& s) noexcept {
  for (int i = 0; i < n; i++) {
    if (s.next()) {
      if (s.getValue().tag == stream_elem::VALUE) {
        co_yield s.getValue();
      } else {
        i--;  // suspension does not carry a value and we want n values
      }
    } else {
      break;
    }
  }
}

Stream<stream_elem> eqv_helper(substitution sub) {
  value u, v;
  if (!eqvValueQueue.empty()) {
    u = eqvValueQueue.front();
    eqvValueQueue.pop();
    if (!eqvValueQueue.empty()) {
      v = eqvValueQueue.front();
      eqvValueQueue.pop();
    } else {
      terminate();
    }
  } else {
    terminate();
  }

  auto unifyRes = unify(u, v, sub);
  if (unifyRes.has_value()) {
    sub = unifyRes.value();
  } else {
    sub = {};
  }
  stream_elem res = {stream_elem::VALUE, sub};
  co_yield res;
}

goal_stream eqv(value u, value v) {
  eqvValueQueue.emplace(u);
  eqvValueQueue.emplace(v);

  return eqv_helper;
}

// Stream<stream_elem> eqv_streamify(stream_elem str) noexcept { co_yield str;
// }

Stream<stream_elem> s_goal_helper(substitution sub) noexcept {
  stream_elem res = {stream_elem::VALUE, sub};
  co_yield res;
}

goal_stream s_goal() {
  // return function pointer instead of lambda
  return s_goal_helper;
}

Stream<stream_elem> u_goal_helper(substitution sub) noexcept {
  stream_elem res = {stream_elem::VALUE, {}};
  co_yield res;
}

goal_stream u_goal() { return u_goal_helper; }

Stream<stream_elem> never_o_goal_helper(substitution sub) noexcept {
  // irregardless of the substitution it produces a suspension
  stream_elem sus = {stream_elem::SUSPEND, {}};
  while (true) {
    co_yield sus;
  }
}

goal_stream never_o_goal() { return never_o_goal_helper; }

Stream<stream_elem> always_o_helper(substitution sub) noexcept {
  stream_elem emptySub = {stream_elem::VALUE, empty_s};
  while (true) {
    co_yield emptySub;
  }
}

goal_stream always_o() { return always_o_helper; }

goal call_fresh(std::string name, goal_abstraction funcGoal) {
  auto var = makeVar(name);
  return funcGoal(var);
}

variable reify_name(int n) {
  std::string number, resString;

  number = std::to_string(n);
  resString = "_";
  resString.append(number);  // e.g. "_1"

  return makeVar(resString);
}

substitution reify_s(value val, substitution sub) {
  auto walked_value = walk(val, sub);

  if (holds_alternative<atom>(walked_value)) {
    auto walked_value_atom = get<atom>(walked_value);
    if (walked_value_atom->tag == atomValue::VAR) {
      int subLength = sub.size();
      variable reifiedVar = reify_name(subLength);
      association resAssoc = {walked_value_atom, reifiedVar};
      sub.push_back(resAssoc);
    }
  }

  if (holds_alternative<vector<atom>>(walked_value)) {
    auto walked_value_pair = get<vector<atom>>(walked_value);
    for (auto elem : walked_value_pair) {
      sub = reify_s(elem, sub);
    }
  }

  return sub;
}

auto reify(value val) {
  auto lambda = [val](substitution sub) -> value {
    value walkedVal = walk_star(val, sub);

    // debug print
    if (holds_alternative<atom>(walkedVal)) {
      auto walked_value_atom = get<atom>(walkedVal);
      std::cout << "reify() walked_value_atom->data: "
                << walked_value_atom->data << std::endl;
    }
    // end debug

    substitution reifiedSub = reify_s(walkedVal, empty_s);
    return walk_star(walkedVal, reifiedSub);
  };

  return lambda;
}

Stream<stream_elem> run_goal(int n, goal_stream goal) {
  Stream<stream_elem> streamRes = goal(empty_s);
  auto resStream = take_inf(n, streamRes);

  while (resStream.next()) {
    co_yield resStream.getValue();
  }
}

Stream<stream_elem> ifte_helper(substitution sub) {
  goal_stream g1 = ifteQueue.front();
  ifteQueue.pop();
  goal_stream g2 = ifteQueue.front();
  ifteQueue.pop();
  goal_stream g3 = ifteQueue.front();
  ifteQueue.pop();
  Stream<stream_elem> sub_inf = g1(sub);

  while (sub_inf.next()) {
    auto fstSub = sub_inf.getValue();

    if (fstSub.tag == stream_elem::VALUE) {  // case (pair? sub_inf)
      auto res = append_map(g2, sub_inf);
      return res;
    }

    // case 'suspension' is implicit (forced)
  }

  return g3(sub);  // case (null? sub_inf)
}

goal_stream ifte(goal_stream g1, goal_stream g2, goal_stream g3) {
  ifteQueue.emplace(g1);
  ifteQueue.emplace(g2);
  ifteQueue.emplace(g3);

  return ifte_helper;
}

Stream<stream_elem> once_helper(substitution sub) {
  goal_stream g1 = onceQueue.front();
  onceQueue.pop();
  Stream<stream_elem> onceStream = g1(sub);

  while (onceStream.next()) {
    auto fstSub = onceStream.getValue();

    if (fstSub.tag == stream_elem::VALUE) {  // case (pair? sub_inf)
      stream_elem res = {stream_elem::VALUE, fstSub.value};
      co_yield res;
    }
  }

  co_yield empty_stream;
}

goal_stream once(goal_stream g1) {
  onceQueue.emplace(g1);
  return once_helper;
}
