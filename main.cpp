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
    if (gen.getValue().tag == stream_elem::VALUE) {
      // std::cout << " " << gen.getValue().value;
    }
  }

  std::cout << "\n\n";

  std::cout << "getNext(100, -10):";
  auto gen2 = getNextInf(100, -10);
  for (int i = 0; i <= 20; ++i) {
    gen2.next();
    if (gen2.getValue().tag == stream_elem::VALUE) {
      // std::cout << " " << gen2.getValue().value;
    }
  }

  std::cout << std::endl;

  std::cout << "getNextFin():";
  auto gen3 = getNextFin();
  for (int i = 0; i <= 5; ++i) {
    gen3.next();
    if (gen3.getValue().tag == stream_elem::VALUE) {
      // std::cout << " " << gen3.getValue().value;
    }
  }

  std::cout << std::endl;

  std::cout << "append_inf():";
  Stream<stream_elem> stream = getNextInf(100, -10);
  auto stream2 = getNextFin();
  auto append_stream = append_inf(stream2, stream);
  for (int i = 0; i <= 20; ++i) {
    if (append_stream.next()) {
      if (append_stream.getValue().tag == stream_elem::VALUE) {
        // std::cout << " " << append_stream.getValue().value;
      }
    }
  }
  std::cout << std::endl;

  std::cout << "take_inf():";
  Stream<stream_elem> infStream = getNextInf(100, -10);
  auto takeStream = take_inf(5, infStream);
  while (takeStream.next()) {
    if (takeStream.getValue().tag == stream_elem::VALUE) {
      // std::cout << " " << takeStream.getValue().value;
    }
  }
  std::cout << std::endl;
}
