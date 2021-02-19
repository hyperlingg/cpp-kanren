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
