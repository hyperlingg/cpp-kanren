/*
Stream data structure for mini-kanren
Author : Jonas Lingg (2021)
*/

#include <coroutine>
#include <iostream>
#include <memory>

#include "Definitions.h"

// source:
// https://www.modernescpp.com/index.php/c-20-an-infinite-data-stream-with-coroutines
template <typename T>
struct Stream {
  struct promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  Stream(handle_type h) : coro(h) {}  // (3)
  handle_type coro;

  ~Stream() {
    if (coro) coro.destroy();
  }
  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;
  Stream(Stream&& oth) noexcept : coro(oth.coro) { oth.coro = nullptr; }
  Stream& operator=(Stream&& oth) noexcept {
    coro = oth.coro;
    oth.coro = nullptr;
    return *this;
  }
  T getValue() { return coro.promise().current_value; }
  bool next() {  // (5)
    coro.resume();
    return not coro.done();
  }

  struct promise_type {
    promise_type() = default;  // (1)

    ~promise_type() = default;

    auto initial_suspend() {  // (4)
      return std::suspend_always{};
    }
    auto final_suspend() { return std::suspend_always{}; }
    auto get_return_object() {  // (2)
      return Stream{handle_type::from_promise(*this)};
    }
    auto return_void() { return std::suspend_never{}; }

    auto yield_value(const T value) {  // (6)
      current_value = value;
      return std::suspend_always{};
    }
    void unhandled_exception() { std::exit(1); }
    T current_value;
  };
};

struct stream_elem {
  enum { SUSPEND, VALUE, ABSTRACTION, EMPTY } tag;
  substitution value;
};

using goal_singleton = std::function<stream_elem(substitution)>;
using goal = std::function<Stream<stream_elem>(substitution)>;
// using goal = variant<goal_singleton, goal_stream>;
using goal_abstraction = std::function<goal(variable)>;

// exemplary stream generator functions for testing purposes
Stream<int> getNextInf(int start = 0, int step = 1) noexcept {
  auto value = start;
  // co_yield value;
  for (int i = 0;; ++i) {
    co_yield value;
    value += step;
  }
}

Stream<int> getNextFin(int start = 0, int end = 10,
                               int step = 1) noexcept {
  auto value = start;
  for (int i = 0; i < end; ++i) {
    co_yield value;
    value += step;
  }
}
