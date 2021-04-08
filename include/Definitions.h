/*
Type definitions for mini-kanren
Author : Jonas Lingg (2021)
*/

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
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

class cons {
  using value_type = variant<cons, atom>;
  using cons_value = shared_ptr<value_type>;

 private:
  cons_value carPtr;
  cons_value cdrPtr;

 public:
  cons_value car() { return carPtr; }

  cons_value cdr() { return cdrPtr; }

  cons(value_type a, value_type b) {
    if (holds_alternative<atom>(a)) {
      atom atmVal = get<atom>(a);
      carPtr = make_shared<value_type>(atmVal);
    } else if (holds_alternative<cons>(a)) {
      cons consVal = get<cons>(a);
      carPtr = make_shared<value_type>(consVal);
    }

    if (holds_alternative<atom>(b)) {
      atom atmVal = get<atom>(b);
      cdrPtr = make_shared<value_type>(atmVal);
    } else if (holds_alternative<cons>(b)) {
      cons consVal = get<cons>(b);
      cdrPtr = make_shared<value_type>(consVal);
    }
  }
};

// TODO this redefinition is inelegant...
using value_type = variant<cons, atom>;
using cons_value = shared_ptr<value_type>;

bool isPair(cons_value val) { return val && holds_alternative<cons>(*val); }

cons getPair(cons_value val) {  // throws exception if !isPair(val)
  try {
    return get<cons>(*val);
  } catch (std::bad_variant_access const& ex) {
    std::cout << ex.what() << ": val contained atom, not cons\n";
    throw;
  }
}

bool isAtom(cons_value val) { return val && holds_alternative<atom>(*val); }

atom getAtom(cons_value val) {  // throws exception if !isAtom(val)
  try {
    return get<atom>(*val);
  } catch (std::bad_variant_access const& ex) {
    std::cout << ex.what() << ": val contained cons, not atom\n";
    throw;
  }
}

// temporary aliases used for integration
using value_list_new = cons;
using value_new = variant<atom, value_list_new>;
