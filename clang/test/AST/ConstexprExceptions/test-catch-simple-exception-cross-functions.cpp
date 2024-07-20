// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

consteval void throw_int(int v) {
  throw v;
}

consteval int test() {
  try {
    throw_int(42);
  } catch (int i) {
    return i * 3;
  }
  return 0;
}

constexpr auto value = test();
static_assert(value == 42*3);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note
