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

using namespace std;

// frame 4
using variable = shared_ptr<string>;
// frames 6,7
using constant = string;
using atom = variant<variable, constant>; // atomic values
using value = variant<variable, constant,
                      vector<atom>>; // a list of values is also a value
using association = pair<variable, value>;

// frames 9,10
// NOTE: can not contain two or more associations with the same first element
// (frame 12); maybe implement as a class where the constructor checks this
// property
using substitution = vector<association>;

// based on frame 11
bool isEmptyS(substitution sub) { return sub.empty(); }

// frame 18
optional<association> assv(value val, substitution sub) {
  if (holds_alternative<variable>(val)) {
    variable var = get<variable>(val);
    for (auto it = sub.begin(); it != sub.end(); ++it) {
      if (it->first == var) {
        return *it;
      }
    }
  }
  return {};
}

value walk(value val, substitution sub) {
  if (holds_alternative<variable>(val)) {
    auto assvOpt = assv(val, sub);
    if (assvOpt.has_value()) {
      auto assvValue = assvOpt.value();
      auto snd = assvValue.second;
      return walk(snd, sub);
    }
  }
  return val;
}

bool occurs(variable var, value val, substitution sub) {
  auto walkRes = walk(val, sub);

  if (holds_alternative<variable>(walkRes)) {
    auto walkVar = get<variable>(walkRes);
    return walkVar == var;
  }

  if (holds_alternative<vector<atom>>(walkRes)) {
    auto walkPair = get<vector<atom>>(walkRes);

    for (auto elem : walkPair) {
      bool occ;
      if (holds_alternative<variable>(elem)) {
        auto valArg = get<variable>(elem);
        occ = occurs(var, valArg, sub);
      }

      // TODO is this even necessary?
      // occur check is interested in vars
      if (holds_alternative<constant>(elem)) {
        auto valArg = get<constant>(elem);
        occ = occurs(var, valArg, sub);
      }

      if (occ) {
        return true;
      }
    }
  }

  return false;
}

optional<substitution> ext_s(variable var, value val, substitution sub) {

  if (occurs(var, val, sub)) {
    return {};
  }

  sub.push_back({var, val});

  return sub;
}
