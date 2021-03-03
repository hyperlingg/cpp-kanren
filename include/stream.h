#include <coroutine>
#include <iostream>
#include <memory>

// source: https://www.modernescpp.com/index.php/c-20-an-infinite-data-stream-with-coroutines
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

Stream<int> append_inf(Stream<int> s, Stream<int> t) noexcept {
  while (s.next()) {
    co_yield s.getValue();
  }

  while (t.next()) {
    co_yield t.getValue();
  }
}

// TODO definition of goals and their application is missing
auto disj(Stream<int> g1, Stream<int> g2) {
  return [&](auto a) { return append_inf(g1, g2); };
}

Stream<int> take_inf(int n, Stream<int> s) {
  for (int i = 0; i < n; i++) {
    s.next();
    co_yield s.getValue();
  }
}