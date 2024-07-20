// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

template <typename T, typename... Args> consteval int test(Args && ... args) {
  try {
    throw T{args...};
  } catch (unsigned v) {
    return static_cast<int>(v) * 2;
  } catch (int v) {
    return v * 3;
  } catch (bool v) {
    return static_cast<int>(v) * 5;
  } catch (...) {
    return -1;
  }
  return 0;
}

static_assert(test<unsigned>(42u) == 84);
static_assert(test<int>(13) == 39);
static_assert(test<bool>(true) == 5);
static_assert(test<long>(42) == -1);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note
