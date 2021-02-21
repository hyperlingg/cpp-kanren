/*
Author : Jonas Lingg (2021)
*/

#include "include/cpp-kanren.h"

int main() {

  // TODO encapsulate in a function, this is unreadable
  // constructing a variable
  atomValue atmvVal = {atomValue::VAR,"x"};
  variable var = make_shared<atomValue>(atmvVal);
  cout << var->data << endl;
}
