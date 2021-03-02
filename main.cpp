/*
Author : Jonas Lingg (2021)
*/

#include "include/cpp-kanren.h"

int main() {
  // TODO encapsulate in a function, this is unreadable
  // constructing a variable
  atomValue atmvVal = {atomValue::VAR, "x"};
  variable var = make_shared<atomValue>(atmvVal);
  cout << var->data << endl;

  // coroutine trial & error
  std::cout << std::endl;

  std::cout << "getNext():";
  auto gen = getNext();
  for (int i = 0; i <= 10; ++i) {
    gen.next();
    std::cout << " " << gen.getValue();  // (7)
  }

  std::cout << "\n\n";

  std::cout << "getNext(100, -10):";
  auto gen2 = getNext(100, -10);
  for (int i = 0; i <= 20; ++i) {
    gen2.next();
    std::cout << " " << gen2.getValue();
  }

  std::cout << std::endl;
}
