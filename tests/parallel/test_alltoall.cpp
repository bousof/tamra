#include <doctest.h>

#include <vector>

#include <tamra/parallel/alltoall.h>
#include <tamra/parallel/alltoallv.h>
#include <tamra/parallel/wrapper.h>

TEST_CASE("[parallel][alltoall] Scalar output is resized and ordered by source") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  std::vector<int> sent(size), expected(size), received;
  for (unsigned p{0}; p < size; ++p) {
    sent[p] = static_cast<int>(100 * rank + p);
    expected[p] = static_cast<int>(100 * p + rank);
  }
  scalar_alltoall(sent, received);
  CHECK(received == expected);
  received.assign(size + 3, -1);
  scalar_alltoall(sent, received);
  CHECK(received == expected);
}

TEST_CASE_TEMPLATE("[parallel][alltoallv] Sparse traffic, offsets and supplied counts", T, int, unsigned, float, double) {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  for (const bool empty : {false, true}) {
    CAPTURE(empty);
    std::vector<std::vector<T>> sent(size), expected_split(size);
    std::vector<T> expected_flat;
    std::vector<int> counts(size), expected_offsets;
    for (unsigned p{0}; p < size; ++p) {
      // Non-symmetric counts catch accidental transposition of rank indices.
      const unsigned send_count = empty ? 0 : (rank + 2 * p + 1) % 4;
      const unsigned recv_count = empty ? 0 : (p + 2 * rank + 1) % 4;
      for (unsigned i{0}; i < send_count; ++i)
        sent[p].push_back(static_cast<T>(100 * rank + 10 * p + i));
      for (unsigned i{0}; i < recv_count; ++i)
        expected_split[p].push_back(static_cast<T>(100 * p + 10 * rank + i));
      expected_offsets.push_back(static_cast<int>(expected_flat.size()));
      expected_flat.insert(expected_flat.end(), expected_split[p].begin(), expected_split[p].end());
      counts[p] = static_cast<int>(recv_count);
    }
    for (const bool supplied_counts : {false, true}) {
      CAPTURE(supplied_counts);
      std::vector<T> received{T{99}};
      std::vector<std::vector<T>> received_split{{T{99}}};
      const auto requested_counts = supplied_counts ? counts : std::vector<int>{};
      const auto offsets = vector_alltoallv(sent, received, requested_counts);
      vector_alltoallv(sent, received_split, requested_counts);
      CHECK(offsets == expected_offsets);
      CHECK(received == expected_flat);
      CHECK(received_split == expected_split);
    }
  }
}

TEST_CASE("[parallel][alltoallv] Matrices preserve rows across uneven messages") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  for (const bool empty : {false, true}) {
    std::vector<std::vector<std::vector<unsigned>>> sent(size);
    std::vector<std::vector<unsigned>> received{{99}}, expected;
    for (unsigned p{0}; p < size; ++p) {
      const unsigned send_count = empty ? 0 : (rank + 2 * p + 1) % 3;
      const unsigned recv_count = empty ? 0 : (p + 2 * rank + 1) % 3;
      for (unsigned i{0}; i < send_count; ++i)
        sent[p].push_back({rank, p, i});
      for (unsigned i{0}; i < recv_count; ++i)
        expected.push_back({p, rank, i});
    }
    matrix_alltoallv(sent, received, 3);
    CHECK(received == expected);
  }
}
