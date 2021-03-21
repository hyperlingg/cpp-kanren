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

  std::cout << "disj():" << std::endl;
  goal_stream disj_goal = disj(s_goal(), u_goal());
  auto resStream = disj_goal(empty_s);
  auto disj_test = disj(s_goal(), u_goal())(empty_s);
  while (resStream.next()) {
    std::cout << " " << resStream.getValue().tag;
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

  std::cout << "eqv problem:";
  auto x = makeVar("x");
  auto y = makeVar("y");

  auto test = eqv(x, y)(empty_s);
  test.next();// NOTE this is essential
  auto streamValueSize = test.getValue().value.size();
  substitution singleton = {{x, y}};

  auto s_goal_test = s_goal();
  auto eval_s_goal = s_goal_test(singleton);
  eval_s_goal.next();
  auto s_goal_value = eval_s_goal.getValue();
  std::cout << "s_goal size" << s_goal_value.value.size() << std::endl;

  std::cout << "streamValueSize: " << streamValueSize << std::endl;
  std::cout << "singleton size: " << singleton.size() << std::endl;
}
