/*
Header-only implementation of mini-kanren
Author : Jonas Lingg (2021)
*/

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "stream.h"

using namespace std;

#define empty_s ((substitution){})

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
      std::swap(t, s);
    } else {
      co_yield s.getValue();
    }
  }

  while (t.next()) {
    co_yield t.getValue();
  }
}

// TODO convert goal_stream to goal by distinguishing variants
goal_stream disj(goal_stream g1, goal_stream g2) {
  return [&](substitution sub) {
    auto stream1 = g1(sub);
    auto stream2 = g2(sub);
    return append_inf(stream1, stream2);
  };
}

Stream<stream_elem> take_inf(int n, Stream<stream_elem>& s) noexcept {
  for (int i = 0; i < n; i++) {
    s.next();
    if (s.getValue().tag == stream_elem::VALUE) {
      co_yield s.getValue();
    } else {
      i--;  // suspension does not carry a value and we want n values
    }
  }
}

// TODO this only doesn't work because of the lambda
// maybe i could achieve the same functionality with a template, e.g.
// eqv<u,v>() : substitution -> goal_stream
goal_stream eqv_stream_lambda(value u, value v) noexcept {
  std::cout << "enter eqv()" << std::endl;

  auto lambda = [u, v](substitution sub) noexcept -> Stream<stream_elem> {
    auto unifyRes = unify(u, v, sub);

    substitution res;
    if (unifyRes.has_value()) {
      res = unifyRes.value();
    }
    stream_elem resElem = {stream_elem::VALUE, res};
    co_yield resElem;
  };

  return lambda;
}

// NOTE one possible solution for eqv; problematic return type: needs to be
// streamified. That is tolerable since eqv is supposed to return either a
// singleton or an empty stream.
auto eqv(value u, value v) {
  return [u, v](substitution sub) -> stream_elem {
    auto unifyRes = unify(u, v, sub);
    std::cout << "eqv() : after unify" << std::endl;
    if (unifyRes.has_value()) {
      std::cout << "eqv() : unify has value" << std::endl;
      sub = unifyRes.value();
    } else {
      sub = {};
      std::cout << "eqv() : unify no value" << std::endl;
    }
    return {stream_elem::VALUE, sub};
  };
}

Stream<stream_elem> eqv_streamify(stream_elem str) noexcept { co_yield str; }

Stream<stream_elem> s_goal_helper(substitution sub) noexcept {
  stream_elem res = {stream_elem::VALUE,
                     sub};  // ...this was enough to remove the segfault. WHY???
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
  stream_elem sus = {stream_elem::SUSPEND, {}};
  while (true) {
    co_yield sus;
  }
}

goal_stream never_o_goal(substitution sub) { return never_o_goal_helper; }

// goal_stream always_o_helper(substitution sub) noexcept {
//   goal_stream res = disj(s_goal(), s_goal());
//   co_yield res;
// }