/*
Author : Jonas Lingg (2021)
*/

#include "include/cpp-kanren.h"

int main() {
  // constructing a variable
  variable var = make_shared<string>("hello");
  cout << *var << endl;
}
