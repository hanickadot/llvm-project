// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

// https://eel.is/c++draft/except.handle#2

template <typename T> consteval T test(T head, auto... tail) {
  const T array[] = {head, tail...};
  try {
    throw array;
  } catch (const T * ptr) {
    return *ptr;
  } catch (...) {
    return -1;
  }
}

constexpr auto r0 = test(0,1,2,3,4);
static_assert(r0 == 0);

constexpr auto r1 = test(2,3,4);
static_assert(r1 == 2);


// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note

