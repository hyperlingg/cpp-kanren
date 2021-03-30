/*
Type definitions for mini-kanren
Author : Jonas Lingg (2021)
*/

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

using namespace std;

// https://en.cppreference.com/w/cpp/language/union
// union-like class
struct atomValue {
  enum { VAR, CONST } tag;
  string data;
};

using atom = shared_ptr<atomValue>;
using variable = atom;
using constant = atom;
using value_list = vector<atom>;  // TODO no real list of list yet
using value = variant<atom,
                      value_list>;  // a list of values is also a value
using association = pair<atom, value>;

// frames 9,10
using substitution = vector<association>;


value car(value_list val) { return val.front(); }

value_list cdr(value_list val) {
  if (!val.empty()) {
    val.erase(val.begin());
    return val;
  } else {
    return {};
  }
}

// we can cons a single atom to either another atom or a list of atoms
value_list cons(atom head, value tail) {
  if (holds_alternative<value_list>(tail)) {
    auto valList = get<value_list>(tail);
    valList.insert(valList.begin(), head);
    return valList;
  } else {
    auto valAtom = get<atom>(tail);
    return {head, valAtom};
  }
}

// NOTE ok, this seems to work as an arbitrary-depth list implementation
struct List {
  variant<atom, unique_ptr<List>> head;
  unique_ptr<List> tail;
};

atomValue val2 = {atomValue::VAR, "a"};
atom val1 = make_shared<atomValue>(val2);

List testList = {val1, make_unique<List>(val1, nullptr)};

List testList2 = {make_unique<List>(val1, make_unique<List>(val1, nullptr)),
                  make_unique<List>(val1, nullptr)};