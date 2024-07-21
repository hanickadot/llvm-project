// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

[[noreturn]] constexpr void increment_exception(int ref) {
  try {
    throw ref;
  } catch (int & ex) {
    ++ex;
    throw; // TODO this puts into stack, but it's immedietely removed!!!
  }
}

constexpr int test() {
  int value = 42;
  try {
    increment_exception(value);
  } catch (int v) {
    return v;
  }
}

constexpr int result = test();
static_assert(result == 43);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note