#pragma once

#include <doctest.h>

#define EPS 1e-12

#define CHECK_VECTOR(A, B)                                    \
  do {                                                        \
    const auto& _a = (A);                                     \
    const auto& _b = (B);                                     \
    REQUIRE(_a.size() == _b.size());                          \
    for (size_t _i = 0; _i < _a.size(); ++_i) {               \
      DOCTEST_INFO("i = " << _i);                             \
      CHECK(_a[_i] == doctest::Approx(_b[_i]).epsilon(EPS));  \
    }                                                         \
  } while (0);
