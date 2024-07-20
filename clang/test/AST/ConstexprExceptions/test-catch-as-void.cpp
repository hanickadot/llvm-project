// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

// TODO URL

constexpr int via_void = 3;
constexpr int via_const_void = 5;
constexpr int via_T = 7;
constexpr int via_const_T = 11;

template <typename T> consteval T test(T value) {
  try {
    throw &value;
  } catch (void * ptr) {
    return *static_cast<T *>(ptr) * via_void;
  } catch (const void * ptr) {
    return *static_cast<const T *>(ptr) * via_const_void;
  }
  //} catch (T * ptr) {
  //  return *ptr * via_T;
  //} catch (const T * ptr) {
  //  return *ptr * via_const_T;
  //} catch (...) {
  //  return -1;
  //}
}

static_assert(test(1) == 1 * via_void);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note

