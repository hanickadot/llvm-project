constexpr int foo() {
  try {
    throw 42;
  } catch (int i) {
    return 2;
  }
}

template <auto> struct identify;

int main() {
  constexpr auto a = foo();
  identify<a> i;
}