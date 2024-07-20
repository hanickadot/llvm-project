// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

struct parent {
  int value;
  explicit constexpr parent(int v) noexcept: value{v} { }
  constexpr virtual int get_value() const noexcept {
    return value;
  }
  constexpr virtual ~parent() = default;
};

consteval int test(void (*fnc)()) {
  try {
    fnc();
    return 0;
  } catch (const parent & par) {
    return par.get_value();
  } catch (...) {
    return -1;
  }
}

constexpr auto r1 = test(+[] { throw parent{42}; });
static_assert(r1 == 42);

struct child: parent {
  explicit constexpr child(int v) noexcept: parent{v} { }
  constexpr int get_value() const noexcept override {
    return value * 3;
  }
};

constexpr auto r2 = test(+[] { throw child{42}; });
static_assert(r2 == 42 * 3);

struct ordinary_child: parent {
  explicit constexpr ordinary_child(int v) noexcept: parent{v} { }
};

// FIXME:
//constexpr auto r3 = test(+[] { throw ordinary_child{21}; });
//static_assert(r3 == 21);


// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note
