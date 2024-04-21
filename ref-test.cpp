consteval int test() {
  //__builtin_rethrow_constexpr_exception(nullptr);
  int a = 42;
  int & b = a;
  __builtin_constexpr_dump(b * b);
  __builtin_constexpr_dump(test);
  //return __builtin_uncaught_constexpr_exceptions();
  return 0;
}

static_assert(test() == 0);
