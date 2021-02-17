/*
relies on C++ 17, compiled with

    clang++-9 -stdlib=libstdc++ -std=gnu++17 main.cpp

author : Jonas Lingg
*/

#include "include/cpp-kanren.h"

int main() {
  // constructing a variable
  variable var = make_shared<string>("hello");
  cout << *var << endl;
}
