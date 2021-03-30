#include "cpp-kanren.h"

// goal car_o(value_list p, atom a) {
//   auto goal_abs = [a, p](variable d) -> goal { return eqv(cons(a, d), p); };
//   return call_fresh("x", goal_abs);
// }

// goal cdr_o(value_list p, value_list d) {
//   auto goal_abs = [p, d](variable a) -> goal { return eqv(cons(a, d), p); };
//   return call_fresh("a", goal_abs);
// }