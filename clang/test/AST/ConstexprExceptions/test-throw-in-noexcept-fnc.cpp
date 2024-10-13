// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

constexpr void throw_exception_here() {
  throw 42;
}

constexpr int test() noexcept {
  // TODO check exception here
  throw_exception_here();
  return 42;
}

constexpr int value = test();

static_assert(value == 42);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note