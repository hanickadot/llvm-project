// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

static constexpr auto via_const_catch = 2;
static constexpr auto via_childs_get_value = 3;
static constexpr auto via_special_child_catch = 5;

struct parent {
  int value;
  explicit constexpr parent(int v) noexcept: value{v} { }
  constexpr virtual int get_value() const noexcept {
    return value;
  }
  constexpr virtual ~parent() = default;
};

struct modifying_child: parent {
  explicit constexpr modifying_child(int v) noexcept: parent{v} { }
  constexpr int get_value() const noexcept override {
    return value * via_childs_get_value;
  }
};

struct ordinary_child: parent {
  explicit constexpr ordinary_child(int v) noexcept: parent{v} { }
};

struct special_child: parent {
  explicit constexpr special_child(int v) noexcept: parent{v} { }
};

consteval int test(void (*fnc)()) {
  int result = 0;
  try {
    fnc();
  } catch (special_child & sch) {
    result = sch.get_value() * via_special_child_catch;
  } catch (const special_child & sch) {
    result = sch.get_value() * via_special_child_catch * via_const_catch;
  } catch (parent & exc) {
    result = exc.get_value();
  } catch (const parent & exc) {
    result = exc.get_value() * via_const_catch;
  }
  return result;
}

constexpr auto r1 = test([] { throw parent{1}; });
static_assert(r1 == 1);

constexpr auto r2 = test([] { throw modifying_child{3}; });
static_assert(r2 == 3 * via_childs_get_value);

// FIXME: same conversion as with reference!
//constexpr auto r3 = test([] { throw ordinary_child{5}; });
//static_assert(r3 == 5);

constexpr auto r4 = test([] { throw special_child{17}; });
static_assert(r4 == 17 * via_special_child_catch); 

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note

