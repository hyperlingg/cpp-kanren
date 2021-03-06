/*
Type definitions for mini-kanren
Author : Jonas Lingg (2021)
*/

#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>
#include <functional>

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
// NOTE: can not contain two or more associations with the same first element
// (frame 12); maybe implement as a class where the constructor checks this
// property
using substitution = vector<association>;
