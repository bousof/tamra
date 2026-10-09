#include <doctest.h>

#include <array>
#include <limits>

#include <tamra/parallel/allreduce.h>
#include <tamra/parallel/wrapper.h>

TEST_CASE_TEMPLATE("[parallel][allreduce] Scalar sums on every rank", T, unsigned, float, double) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  T value = static_cast<T>(rank + 1);
  T result{};
  scalar_sum_allreduce(value, result);
  CHECK(result == doctest::Approx(static_cast<double>(size) * (size + 1) / 2));

  // Scalar input is passed by value, so using the same variable is supported.
  scalar_sum_allreduce(value, value);
  CHECK(value == result);
}

TEST_CASE_TEMPLATE("[parallel][allreduce] Floating extrema and signed sums", T, float, double) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  const T value = static_cast<T>(rank) - static_cast<T>(size) + T{0.25};
  T minimum{}, maximum{}, sum{};
  scalar_min_allreduce(value, minimum);
  scalar_max_allreduce(value, maximum);
  scalar_sum_allreduce(value, sum);
  CHECK(minimum == doctest::Approx(0.25 - size));
  CHECK(maximum == doctest::Approx(-0.75));
  CHECK(sum == doctest::Approx(static_cast<double>(size) * (size - 1) / 2
                             - static_cast<double>(size) * size + 0.25 * size));
}

TEST_CASE("[parallel][allreduce] Logical AND includes nonzero ranks") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  bool result = false;
  bool_and_allreduce(true, result);
  CHECK(result);
  bool_and_allreduce(rank != size - 1, result);
  CHECK_FALSE(result);
  bool_and_allreduce(false, result);
  CHECK_FALSE(result);
}

TEST_CASE_TEMPLATE("[parallel][allreduce] Array sums and zero counts", T, unsigned, float, double) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  const std::array<T, 3> values{{static_cast<T>(rank + 1), T{2}, T{0}}};
  std::array<T, 3> result{{T{9}, T{9}, T{9}}};
  array_sum_allreduce(values.data(), result.data(), values.size());
  CHECK(result[0] == doctest::Approx(static_cast<double>(size) * (size + 1) / 2));
  CHECK(result[1] == static_cast<T>(2 * size));
  CHECK(result[2] == T{0});

  T single{};
  array_sum_allreduce(values.data(), &single, 1);
  CHECK(single == result[0]);

  T sentinel{7};
  array_sum_allreduce<T>(nullptr, &sentinel, 0);
  array_sum_allreduce<T>(nullptr, nullptr, 0);
  CHECK(sentinel == T{7});
}

#ifdef USE_MPI
TEST_CASE_TEMPLATE("[parallel][allreduce] Oversized arrays fail before MPI", T, unsigned, float, double) {
  const std::size_t count = static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
  // All ranks take the same exception path; no oversized allocation is needed.
  CHECK_THROWS_AS(array_sum_allreduce<T>(nullptr, nullptr, count), std::length_error);
}
#endif
