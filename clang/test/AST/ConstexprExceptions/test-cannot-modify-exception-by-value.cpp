// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

struct copied_object {
  constexpr copied_object(int v): value{v} {
    __constexpr_print("copied_object: constructed");
  }
  constexpr copied_object(const copied_object & orig): value{orig.value} {
    __constexpr_print("copied_object: copied"); 
  }
  constexpr copied_object(copied_object && orig): value{orig.value} {
    __constexpr_print("copied_object: moved");
  }
  constexpr ~copied_object() {
    __constexpr_print("copied_object: destroyed");
  }
  int value;
};

struct same_object {
  constexpr same_object(int v): value{v} {
    __constexpr_print("same_object: constructed");
  }
  constexpr same_object(const same_object & orig): value{orig.value} {
    __constexpr_print("same_object: copied"); 
  }
  constexpr same_object(same_object && orig): value{orig.value} {
    __constexpr_print("same_object: moved");
  }
  constexpr ~same_object() {
    __constexpr_print("same_object: destroyed");
  }
  int value;
};

constexpr void increment_exception(void (*fnc)(), void * & address) {
  try {
    fnc();
  } catch (int & ex) { // taking by reference
    address = __builtin_addressof(ex);
    ++ex;
    throw;
  } catch (long ex) { // taking by value
    address = __builtin_addressof(ex);
    ++ex;
    throw;
  } catch (copied_object ex) {
    address = __builtin_addressof(ex);
    ++ex.value;
    throw;
  } catch (same_object & ex) {
    address = __builtin_addressof(ex);
    ++ex.value;
    throw;
  }
}

constexpr int test(void (*fnc)()) {
  void * address = nullptr;
  try {
    increment_exception(fnc, address);
  } catch (int v) {
    constexpr_assert(address != __builtin_addressof(v), "should be a copy");
    return v;
  } catch (long v) {
    constexpr_assert(address != __builtin_addressof(v), "should be a copy");
    return static_cast<int>(v);
  } catch (const copied_object & ex) {
    constexpr_assert(address != __builtin_addressof(ex), "should be a copy");
    return ex.value;
  } catch (const same_object & ex) {
    constexpr_assert(address == __builtin_addressof(ex), "must be same object");
    return ex.value;
  }
  return -1;
}

constexpr int incremented = test([]{throw int{42};});
static_assert(incremented == 43);

constexpr int same = test([]{throw long{40};});
static_assert(same == 40);

constexpr int should_be_same_value = test([]{throw copied_object{97};});
static_assert(should_be_same_value == 97);
// CHECK: copied_object: copied
// CHECK-NOT: copied_object: moved

constexpr int should_be_modified = test([]{throw same_object{11};});
static_assert(should_be_modified == 12);
// CHECK-NOT: same_object: copied
// CHECK-NOT: same_object: moved

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note