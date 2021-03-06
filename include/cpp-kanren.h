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
  if (occurs(var, val, sub)) {
    return {};
  }

  sub.push_back({var, val});

  return sub;
}

optional<substitution> unify(value u, value v, substitution sub) {
  auto uWalk = walk(u, sub);
  auto vWalk = walk(v, sub);

  if (uWalk == vWalk) {
    return sub;
  }

  if (holds_alternative<atom>(uWalk)) {
    auto uAtom = get<atom>(uWalk);
    if (uAtom->tag == atomValue::VAR) {
      return ext_s(uAtom, v, sub);
    }
  }

  if (holds_alternative<atom>(vWalk)) {
    auto vAtom = get<atom>(vWalk);
    if (vAtom->tag == atomValue::VAR) {
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

goal disj(goal g1, goal g2) {
  return [&](substitution sub) {
    auto stream1 = g1(sub);
    auto stream2 = g2(sub);
    return append_inf(stream1, stream2);
  };
}

Stream<stream_elem> take_inf(int n, Stream<stream_elem>& s) {
  for (int i = 0; i < n; i++) {
    s.next();
    if (s.getValue().tag == stream_elem::VALUE) {
      co_yield s.getValue();
    } else {
      i--;  // suspension does not carry a value and we want n values
    }
  }
}

auto eqv(value u, value v) {
  return [&](substitution sub) -> Stream<stream_elem> {
    auto unifyRes = unify(u, v, sub);
    if (unifyRes.has_value()) {
      sub = unifyRes.value();
    } else {
      sub = {};
    }
    co_yield {stream_elem::VALUE, sub};
  };
}

auto s_goal(substitution sub) {
  return [&]() -> Stream<stream_elem> { co_yield {stream_elem::VALUE, sub}; };
}

auto u_goal(substitution sub) {
  return [&]() -> Stream<stream_elem> { co_yield {stream_elem::VALUE, {}}; };
}
