// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

// TODO URL

consteval int test_nullptr() {
  try {
    throw nullptr;
  } catch (const int * ex) {
    return true;
  } catch (...) {
    return false;
  }
}

static_assert(test_nullptr());

consteval int test_zero() {
  try {
    throw 0;
  } catch (const int * ex) {
    return false;
  } catch (...) {
    return true;
  }
}

static_assert(test_zero());

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note

