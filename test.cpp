struct my_exception {
  int value;
};

constexpr int test0a() {
  try {
    throw new int{42};
  } catch (int * v) {
    int r = *v;
    delete v;
    return r;
  }
}

static_assert(test0a() == 42);

constexpr int test0() {
  try {
    if (true) {
      throw 42;
      return 3;
    }
    return 1;
  } catch (...) {
    return 2;
  }
}

static_assert(test0() == 2);

constexpr int test0b() {
  try {
    throw 42;
  } catch (int i) {
    return i;
  } catch (...) {
    return 11;
  }
}

static_assert(test0b() == 42);

constexpr int test1() {
  try {
    throw my_exception{42};
  } catch (int i) {
    return 1;
  } catch (const my_exception & e) {
    return 3;
  } catch (...) {
    return 4;
  }
  return 14;
}

//static_assert(test1() == 3);

constexpr int test2() {
  throw 42;
  return 1;
}

constexpr int test3() {
  try {
    test2();
    return 3;
  } catch (...) {
    return 2;
  }
}

static_assert(test3() == 2);


template <auto> struct identify;


int main() {
  //constexpr auto a = test0b();
  //identify<a> i;
}