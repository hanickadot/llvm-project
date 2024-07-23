// RUN: not %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

[[noreturn]] constexpr auto create(int v) -> int {
  throw v;
}

constexpr int convert(int x) {
  return x;
}

// CHECK: error: constexpr variable 'value1' must be initialized by a constant expression
// CHECK: note: unhandled exception of type 'int' with content 42 thrown from here
constexpr auto value1 = convert(create(42));

struct wrapper {
  int value;
};

// CHECK: error: constexpr variable 'value2' must be initialized by a constant expression
// CHECK: note: unhandled exception of type 'int' with content 56 thrown from here
constexpr auto value2 = wrapper{create(56)};

// CHECK: error: constexpr variable 'value3' must be initialized by a constant expression
// CHECK: note: unhandled exception of type 'int' with content 32 thrown from here
constexpr auto value3 = wrapper(create(32));

// CHECK: error: constexpr variable 'value4' must be initialized by a constant expression
// CHECK: note: unhandled exception of type 'int' with content 4 thrown from here
constexpr auto value4 = (1,2,3,create(4));

constexpr int fnc() {
  const auto v = create(1);
  return v;
}

constexpr auto value5 = fnc();