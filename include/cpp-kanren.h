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
// using variable = shared_ptr<string>;
// frames 6,7
// using constant = string;
// using atom = variant<variable, constant>; // atomic values

// https://en.cppreference.com/w/cpp/language/union
// union-like class
struct atomValue {
  enum { VAR, CONST } tag;
  string data;
};

using atom = shared_ptr<atomValue>;
using variable = atom;
using constant = atom;
using value_list = vector<atom>; // TODO no real list of list yet
using value = variant<atom,
                      value_list>; // a list of values is also a value
using association = pair<atom, value>;

// frames 9,10
// NOTE: can not contain two or more associations with the same first element
// (frame 12); maybe implement as a class where the constructor checks this
// property
using substitution = vector<association>;

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
