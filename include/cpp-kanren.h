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
using value = vector<variant<variable, string>>;
using associate = variant<variable, value>;
using association = std::pair<variable, associate>;

// frames 9,10
// NOTE: can not contain two or more associations with the same first element
// (frame 12); maybe implement as a class where the constructor checks this
// property
using substitution = vector<association>;

// based on frame 11
bool isEmptyS(substitution sub) { return sub.empty(); }

// frame 18
optional<association> assv(value val, substitution sub) {

  if (val.size() == 1) {
    auto fst = val.front();
    if (holds_alternative<variable>(fst)) {
      // get shared pointer for comparison
      variable var = get<variable>(fst);

      for (auto it = sub.begin(); it != sub.end(); ++it) {
        if (it->first == var) {
          return *it;
        }
      }
    }
  }

  return {};
}