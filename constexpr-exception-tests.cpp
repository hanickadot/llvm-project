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

constexpr int passing_reference_to_exception() {
  try {
    throw 21;
    return 1;
  } catch (const int & i) {
    return i;
  } catch (...) {
    return 3;
  }
  return 4;
}

static_assert(passing_reference_to_exception() == 21);
