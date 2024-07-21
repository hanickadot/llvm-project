// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

struct hanaxception {
  int v;
};

struct checker {
  int value;
  constexpr checker(int v) try : value{v} {
    if (v > 10) {
      throw hanaxception{v};
    }
  } catch (const hanaxception h) {
    
  }
};

constexpr int test() {
  auto c = checker{11};
  return 42;
}

constexpr int constructor_test = test();

static_assert(constructor_test == 42);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note