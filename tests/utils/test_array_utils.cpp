#include <doctest.h>

#include <limits>
#include <vector>

#include <tamra/utils/array_utils.h>

TEST_CASE("[utils][array] Inclusive and exclusive cumulative sums") {
  const std::vector<int> counts{2, 0, 3, 1};
  std::vector<int> offsets{99};
  CHECK(cumulative_sum(counts, offsets) == 6);
  CHECK(offsets == std::vector<int>{2, 2, 5, 6});
  CHECK(cumulative_sum(counts, offsets, true) == 6);
  CHECK(offsets == std::vector<int>{0, 2, 2, 5});
  CHECK(cumulative_sum(std::vector<int>{}, offsets, true) == 0);
  CHECK(offsets.empty());
}

TEST_CASE("[utils][array] Cumulative sums preserve fractional loads") {
  const std::vector<double> loads{0.25, 0.5, 1.75};
  std::vector<double> sums;
  CHECK(cumulative_sum(loads, sums) == doctest::Approx(2.5));
  CHECK(sums == std::vector<double>{0.25, 0.75, 2.5});
  CHECK(cumulative_sum(loads, sums, true) == doctest::Approx(2.5));
  CHECK(sums == std::vector<double>{0., 0.25, 0.75});
}

TEST_CASE("[utils][array] Unsigned cumulative sums retain their full range") {
  const unsigned large = static_cast<unsigned>(std::numeric_limits<int>::max()) + 1u;
  std::vector<unsigned> sums;
  CHECK(cumulative_sum(std::vector<unsigned>{large, 1u}, sums) == large + 1u);
  CHECK(sums == std::vector<unsigned>{large, large + 1u});
}

TEST_CASE("[utils][array] Splitting preserves empty buckets and input values") {
  const std::vector<int> values{3, 5, 7};
  std::vector<std::vector<int>> result{{99}, {100}};
  split(values, result, {0, 0, 2, 3});
  CHECK(result == std::vector<std::vector<int>>{{}, {3, 5}, {7}, {}});
  CHECK(values == std::vector<int>{3, 5, 7});
  split(std::vector<int>{}, result, {0, 0});
  CHECK(result == std::vector<std::vector<int>>(2));
  split(std::vector<int>{}, result, {});
  CHECK(result.empty());
}

TEST_CASE("[utils][array] Linspace handles empty singleton and reversed intervals") {
  CHECK(linspace(2., 5., 0).empty());
  CHECK(linspace(2., 5., 1) == std::vector<double>{2.});
  CHECK(linspace(2., 5., 2) == std::vector<double>{2., 5.});
  CHECK(linspace(-1., 1., 5) == std::vector<double>{-1., -0.5, 0., 0.5, 1.});
  CHECK(linspace(1., -1., 3) == std::vector<double>{1., 0., -1.});
}

TEST_CASE("[utils][array] Concatenation and Boolean predicates include empty inputs") {
  CHECK(concatenate(std::vector<double>{1., 2.}, std::vector<double>{3.})
        == std::vector<double>{1., 2., 3.});
  CHECK(concatenate(std::vector<int>{}, std::vector<int>{4}) == std::vector<int>{4});
  CHECK(concatenate(std::vector<int>{4}, std::vector<int>{}) == std::vector<int>{4});
  CHECK(all({}));
  CHECK_FALSE(any({}));
  CHECK(all({true, true}));
  CHECK_FALSE(all({true, false}));
  CHECK(any({false, true}));
  CHECK_FALSE(any({false, false}));
}
