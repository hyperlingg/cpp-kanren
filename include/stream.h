#include <coroutine>
#include <iostream>
#include <memory>

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
  enum { SUSPEND, VALUE } tag;
  int value;  // TODO -> substitution
};

// exemplary stream generator functions for testing purposes
Stream<stream_elem> getNextInf(int start = 0, int step = 1) noexcept {
  auto value = start;
  co_yield {stream_elem::SUSPEND, 0};
  for (int i = 0;; ++i) {
    co_yield {stream_elem::VALUE, value};
    value += step;
  }
}

Stream<stream_elem> getNextFin(int start = 0, int end = 10,
                               int step = 1) noexcept {
  auto value = start;
  for (int i = 0; i < end; ++i) {
    co_yield {stream_elem::VALUE, value};
    value += step;
  }
}

// TODO encode suspensions
Stream<stream_elem> append_inf(Stream<stream_elem>& s,
                               Stream<stream_elem>& t) noexcept {
  while (s.next()) {
    if (s.getValue().tag ==
        stream_elem::SUSPEND) {  // if suspension -> swap streams
      std::swap(t, s);
    } else {
      co_yield s.getValue();
    }
  }

  while (t.next()) {
    co_yield t.getValue();
  }
}

// TODO definition of goals and their application is missing
auto disj(Stream<stream_elem> g1, Stream<stream_elem> g2) {
  return [&](auto a) { return append_inf(g1, g2); };
}

Stream<stream_elem> take_inf(int n, Stream<stream_elem>& s) {
  for (int i = 0; i < n; i++) {
    s.next();
    if (s.getValue().tag == stream_elem::VALUE) {
      co_yield s.getValue();
    } else {
      i--;  // suspension does not carry a value and we want n values
    }
  }
}
