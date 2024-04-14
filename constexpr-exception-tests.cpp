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

//static_assert(passing_reference_to_exception() == 21);

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
      throw i;
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

