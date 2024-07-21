// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

// https://eel.is/c++draft/except.handle#2

template <typename T> consteval T test(T head, auto... tail) {
  const T array[] = {head, tail...};
  try {
    throw array;
  } catch (const T (&arr)[5]) {
    return -2;
  } catch (const T * ptr) {
    return *ptr;
  } catch (...) {
    return -1;
  }
}

constexpr auto r0 = test(1,2,3,4,5,6);
static_assert(r0 == 1);

constexpr auto r1 = test(1,2,3,4,5);
static_assert(r1 == 1);

constexpr auto r2 = test(1,2,3,4);
static_assert(r2 == 1);

constexpr auto r3 = test(7,1,2,3,4,5,6);
static_assert(r3 == 7);

constexpr auto r4 = test(8,1,2,3,4,5);
static_assert(r4 == 8);

constexpr auto r5 = test(9,1,2,3,4);
static_assert(r5 == 9);



// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note

