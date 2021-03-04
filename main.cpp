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

  // coroutine trial & error:
  std::cout << std::endl;

  std::cout << "getNext():";
  auto gen = getNextInf();
  for (int i = 0; i <= 10; ++i) {
    gen.next();
    std::cout << " " << gen.getValue();  // (7)
  }

  std::cout << "\n\n";

  std::cout << "getNext(100, -10):";
  auto gen2 = getNextInf(100, -10);
  for (int i = 0; i <= 20; ++i) {
    gen2.next();
    std::cout << " " << gen2.getValue();
  }

  std::cout << std::endl;

  std::cout << "getNextFin():";
  auto gen3 = getNextFin();
  for (int i = 0; i <= 5; ++i) {
    gen3.next();
    std::cout << " " << gen3.getValue();
  }

  std::cout << std::endl;

  std::cout << "append_inf():";
  Stream<int> stream = getNextInf(100, -10);
  auto stream2 = getNextFin();
  auto append_stream = append_inf(stream2, stream);
  for (int i = 0; i <= 20; ++i) {
    if (append_stream.next()) {
      std::cout << " " << append_stream.getValue();
    }
  }
  std::cout << std::endl;

  std::cout << "take_inf():";
  Stream<int> infStream = getNextInf(100, -10);
  auto takeStream = take_inf(5, infStream);
  while (takeStream.next()) {
    std::cout << " " << takeStream.getValue();
  }
  std::cout << std::endl;
}
