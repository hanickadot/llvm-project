// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

consteval int test() {
  try {
    throw 42;
  } catch (unsigned) {
    return 1;
  } catch (int i) {
    return i * 2;
  } catch (...) {
    return 3;
  }
  return 0;
}

constexpr auto value = test();
static_assert(value == 84);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note
