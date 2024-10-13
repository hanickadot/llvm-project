// RUN: %clang_cc1 -std=c++26 %s -o - -fcxx-exceptions 2>&1 -verify=expected %s

struct exception_from_destructor {
  constexpr const char * what() const noexcept {
    return "exception_from_destructor";
  }
};

struct throws_in_destructor {
  constexpr ~throws_in_destructor() noexcept(false) {
    throw exception_from_destructor{};
  }
};

constexpr int test(bool throw_it) {
  try {
    auto obj = throws_in_destructor{}; // expected-error {{exception thrown out of an object destructor while unrolling another exception}}
    if (throw_it) {
      throw 42;
    }
    return 1;
  } catch (exception_from_destructor) {
    return 2;
  }
}

constexpr int v0 = test(false);
static_assert(v0 == 2);

constexpr int v1 = test(true); // expected-error {{constexpr variable 'v1' must be initialized by a constant expression}} \
// expected-note {{in call to 'test(true)'}}

constexpr int v2 = test(false);
static_assert(v2 == 2);
