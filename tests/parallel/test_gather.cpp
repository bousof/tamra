#include <doctest.h>

#include <vector>

#include <tamra/parallel/allgather.h>
#include <tamra/parallel/allgatherv.h>
#include <tamra/parallel/gather.h>
#include <tamra/parallel/wrapper.h>

TEST_CASE("[parallel][gather] Scalars arrive in rank order at each root") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  for (unsigned root{0}; root < size; ++root) {
    CAPTURE(root);
    double value = 0.5 + rank;
    std::vector<double> received{99., 100.}, expected;
    for (unsigned p{0}; p < size; ++p)
      expected.push_back(0.5 + p);
    scalar_gather(value, received, root, rank, size);
    if (rank == root)
      CHECK(received == expected);
    else
      CHECK(received == std::vector<double>{99., 100.});
    CHECK(value == 0.5 + rank);
  }
}

TEST_CASE_TEMPLATE("[parallel][allgather] Scalars resize and replace output", T, int, unsigned) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  std::vector<T> received(size + 3, T{99}), expected;
  for (unsigned p{0}; p < size; ++p)
    expected.push_back(static_cast<T>(3 * p + 1));
  scalar_allgather(static_cast<T>(3 * rank + 1), received, size);
  CHECK(received == expected);
}

TEST_CASE("[parallel][allgather] Rectangular matrices retain rank and row order") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  const std::vector<std::vector<unsigned>> sent{{10 * rank, 10 * rank + 1},
                                               {10 * rank + 2, 10 * rank + 3}};
  std::vector<std::vector<unsigned>> received{{99}}, expected;
  for (unsigned p{0}; p < size; ++p) {
    expected.push_back({10 * p, 10 * p + 1});
    expected.push_back({10 * p + 2, 10 * p + 3});
  }
  matrix_allgather(sent, received, size);
  CHECK(received == expected);
}

TEST_CASE("[parallel][allgather] Empty matrices and zero-column rows") {
  const unsigned size = mpi_size();
  INFO("rank = " << mpi_rank());
  std::vector<std::vector<unsigned>> received{{99}};
  matrix_allgather(std::vector<std::vector<unsigned>>{}, received, size);
  CHECK(received.empty());
  matrix_allgather(std::vector<std::vector<unsigned>>(2), received, size);
  CHECK(received == std::vector<std::vector<unsigned>>(2 * size));
}

TEST_CASE_TEMPLATE("[parallel][allgatherv] Uneven buffers and exclusive offsets", T, unsigned, double) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  // Include an empty sender among uneven messages, then an all-empty exchange.
  for (unsigned mode{0}; mode < 3; ++mode) {
    CAPTURE(mode);
    std::vector<T> sent, expected, received{T{99}};
    std::vector<int> expected_offsets;
    for (unsigned p{0}; p < size; ++p) {
      expected_offsets.push_back(static_cast<int>(expected.size()));
      const unsigned count = mode == 0 ? p + 1 : mode == 1 ? p : 0;
      for (unsigned i{0}; i < count; ++i) {
        const T value = static_cast<T>(100 * p + i + 1);
        expected.push_back(value);
        if (p == rank)
          sent.push_back(value);
      }
    }
    const auto offsets = vector_allgatherv(sent, received, size);
    CHECK(offsets == expected_offsets);
    CHECK(received == expected);
  }
}
