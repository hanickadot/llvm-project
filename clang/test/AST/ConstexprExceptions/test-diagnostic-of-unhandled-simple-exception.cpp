// RUN: not %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

consteval int test() {
  throw 42;
  return 1;
}

static_assert(test());

// CHECK: error: static assertion expression is not an integral constant expression
// CHECK: unhandled exception of type 'int' with content: 42
