// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

#pragma clang diagnostic ignored "-Wunused-value"

constexpr int throw_it() {
  return (1, throw 2, 3);
}

constexpr int catch_it() {
  try {
    return throw_it();
  } catch (int v) {
    return v;
  }
}

static_assert(catch_it() == 2);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note