// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 | FileCheck --allow-empty %s

struct checker {
  int & counter;
  
  constexpr ~checker() {
    ++counter;
    
  }
};

constexpr int test() {
  int counter = 0;
  {
    try {
      auto c1 = checker{counter};
      throw 42;
    } catch (...) {
      return counter * 7;
    }
  }
  
  return counter * 3;
}

constexpr int destruction_counter = test();

static_assert(destruction_counter == 7);

// CHECK-NOT: error
// CHECK-NOT: warning
// CHECK-NOT: note