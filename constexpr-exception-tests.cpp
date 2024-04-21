struct sink { };

constexpr bool operator==(sink, sink) {
  return false;
}

template <typename T> struct type_of_t {
  constexpr friend bool operator==(type_of_t<T>, type_of_t<T>) {
    return true;
  }
  constexpr explicit(false) operator sink() const noexcept {
    return {};
  }
};

template <typename T> constexpr auto type = type_of_t<T>{};
template <auto V> constexpr auto type_of = type_of_t<decltype(V)>{};

// TEST CODE STARTS HERE

constexpr int expression_with_only_throw() {
  throw 42;
}

constexpr int simple_test() {
  return 1;
}

static_assert(simple_test() == 1);

constexpr int local_throw_and_generic_catch() {
  try {
    throw 42;
    return 1;
  } catch (...) {
    return 2;
  }
  return 3;
}

static_assert(local_throw_and_generic_catch() == 2);

constexpr int local_throw_and_specific_catch() {
  try {
    throw 42;
    return 1;
  } catch (int i) {
    return i;
  } catch (...) {
    return 3;
  }
  return 4;
}

static_assert(local_throw_and_specific_catch() == 42);

constexpr int passing_argument(int a) {
  try {
    throw a;
    return 1;
  } catch (int i) {
    return i;
  } catch (...) {
    return 3;
  }
  return 4;
}

static_assert(passing_argument(42) == 42);

constexpr int passing_result_of_expression(int a, int b) {
  try {
    throw a + b;
    return 1;
  } catch (int i) {
    return i;
  } catch (...) {
    return 3;
  }
  return 4;
}

static_assert(passing_result_of_expression(3,5) == 8);

struct type1 {
  constexpr int get_value() const noexcept {
    return 1;
  }
};

struct type2 {
  constexpr int get_value() const noexcept {
    return 2;
  }
};

constexpr int complex_type_A() {
  try {
    throw type1();
    return -1;
  } catch (type1) {
    return 1;
  } catch (type2) {
    return 2;
  }
  return 3;
}

constexpr int complex_type_B() {
  try {
    throw type1();
    return -1;
  } catch (type2) {
    return 2;
  } catch (type1) {
    return 1;
  }
  return 3;
}

static_assert(complex_type_A() == 1);
static_assert(complex_type_B() == 1);

template <int I> struct typeN {
  constexpr int get_value() const noexcept {
    return I;
  }
};

constexpr int templated_type() {
  try {
    throw typeN<0>();
    return -1;
  } catch (type1) {
    return 1;
  } catch (type2) {
    return 2;
  } catch (...) {
    return 0;
  }
}

static_assert(templated_type() == 0);

constexpr int templated_type_catched() {
  try {
    throw typeN<42>();
    return -1;
  } catch (typeN<42>) {
    return 1;
  } catch (type2) {
    return 2;
  } catch (...) {
    return 0;
  }
}

static_assert(templated_type_catched() == 1);


constexpr int templated_type_catched_without_catchall() {
  try {
    throw typeN<0>();
    return -1;
  } catch (typeN<0>) {
    return 1;
  } catch (type2) {
    return 2;
  }
  return 3;
}

static_assert(templated_type_catched_without_catchall() == 1);

constexpr int different_types_thowing(int t) {
  try {
    if (t == 0) {
      throw typeN<0>();
    } else if (t == 1) {
      throw typeN<1>();
    } else {
      throw typeN<2>();
    }
    return -1;
  } catch (typeN<0> a) {
    return a.get_value();
  } catch (typeN<1> a) {
    return a.get_value();
  } catch (typeN<2> a) {
    return a.get_value();
  }
}

static_assert(different_types_thowing(0) == 0);
static_assert(different_types_thowing(1) == 1);
static_assert(different_types_thowing(2) == 2);

struct complex_type {
  int * a;
  int b;
};

constexpr int throwing_complex_type() {
  try {
    int * a = new int{42};
    throw complex_type{.a = a, .b = 2};
  } catch (complex_type ct) {
    int v = *ct.a;
    delete ct.a;
    return v;
  }
  return 3;
}

static_assert(throwing_complex_type() == 42);

//constexpr int passing_reference_to_exception() {
//  try {
//    throw 21;
//    return 1;
//  } catch (const int & i) {
//    return i;
//  } catch (...) {
//    return 3;
//  }
//  return 4;
//}

//static_assert(passing_reference_to_exception() == 21); // FAILING

constexpr int rethrowing(int i) {
  try {
    throw i;
  } catch (int i) {
    try {
      throw i;
    } catch (int j) {
      return j;
    }
  }
  return -1;
}

static_assert(rethrowing(31) == 31);

constexpr int rethrowing_without_name(int i) {
  try {
    throw i;
  } catch (...) {
    try {
      throw;
    } catch (int j) {
      return j;
    }
  }
  return -1;
}

static_assert(rethrowing_without_name(33) == 33);

constexpr int throwing_from_other_statements(int i) {
  try {
    if (i > 0) {
      for (int j = 0; j != 5; ++j) {
        throw i;
      }
    } else {
      throw i;
    }
    return -1;
  } catch (int i) {
    return i;
  } catch (...) {
    return 42;
  }
}

static_assert(throwing_from_other_statements(1) == 1);
static_assert(throwing_from_other_statements(-1) == -1);

constexpr int weird_switch(int i) {
  try {
    switch (i) {
      case 0:
        throw 41;
      case 1:
        throw 42;
      default:
        throw 43;
    }
    return -1;
  } catch (int e) {
    return e;
  }
}

static_assert(weird_switch(0) == 41);
static_assert(weird_switch(1) == 42);
static_assert(weird_switch(2) == 43);

constexpr int * with_allocation(int v) {
  try {
    throw new int{v};
  } catch (int * ptr) {
    return ptr;
  }
}

constexpr int result_from_allocation(int v) {
  const int * ptr = with_allocation(v);
  int r = *ptr;
  delete ptr;
  return r;
}

static_assert(result_from_allocation(1) == 1);
static_assert(result_from_allocation(2) == 2);
static_assert(result_from_allocation(3) == 3);

struct complex_object {
  int a;
  int b;
  int c;
};

constexpr auto throwing_complex_object() {
  try {
    throw complex_object{1,2,3};
  } catch (complex_object obj) {
    return obj;
  }
}


static_assert(type_of<throwing_complex_object()> == type<complex_object>);
static_assert(throwing_complex_object().a == 1);
static_assert(throwing_complex_object().b == 2);
static_assert(throwing_complex_object().c == 3);

constexpr int throwing_reference_matches_first_a(const int & r) {
  try {
    throw r;
  } catch (const int & a) {
    return 1;
  } catch (int b) {
    return 2;
  }
  return 3;
}

static_assert(throwing_reference_matches_first_a(1) == 1); /// FAILING

constexpr int throwing_reference_matches_first_b(const int & r) {
  try {
    throw r;
  } catch (int b) {
    return 2;
  } catch (const int & a) {
    return 1;
  }
  return 3;
}

static_assert(throwing_reference_matches_first_b(1) == 2);

struct base {
  
};

struct child: base {

};

constexpr int throw_child() {
  try {
    throw child{};
  } catch (const base & a) {
    return 1;
  } catch (...) {
    return 2;
  }
  return 3;
}

static_assert(throw_child() == 1);

struct baseA { };
struct baseB { };
struct multi_child: baseA, baseB { };

constexpr int throw_multi_child() {
  try {
    throw multi_child{};
  } catch (const baseA & a) {
    return 1;
  } catch (...) {
    return 2;
  }
  return 3;
}

static_assert(throw_multi_child() == 1); 

struct hidden_base { int x; };
struct public_base { int y; };

struct multi_child_with_hidden_base: private hidden_base, public_base {
  constexpr multi_child_with_hidden_base(int a, int b): hidden_base{a}, public_base{b} { }
};

constexpr int throw_multi_child_with_hidden_base(bool placeholder) {
  try {
    throw multi_child_with_hidden_base{13,14};
  } catch (const hidden_base & v) {
    return 2; // will never match!
  } catch (const public_base & w) {
    if (placeholder) {
      return 3;
    }
    return w.y; // crash here when placeholder = true
  } catch (...) {
    return 4;
  }
  return 5;
}

static_assert(throw_multi_child_with_hidden_base(true) == 3); 
static_assert(throw_multi_child_with_hidden_base(false) == 14); 
