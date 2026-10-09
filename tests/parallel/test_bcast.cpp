#include <doctest.h>

#include <string>
#include <vector>

#include <tamra/parallel/bcast.h>
#include <tamra/parallel/wrapper.h>

TEST_CASE("[parallel][bcast] Boolean and unsigned scalars from every root") {
  const unsigned rank = mpi_rank(), size = mpi_size();
  INFO("rank = " << rank);
  for (unsigned root{0}; root < size; ++root) {
    CAPTURE(root);
    for (const bool expected : {false, true}) {
      bool value = rank == root ? expected : !expected;
      scalar_bcast(value, root);
      CHECK(value == expected);
    }
    unsigned value = rank == root ? root + 7 : 0;
    scalar_bcast(value, root);
    CHECK(value == root + 7);
  }
}

TEST_CASE_TEMPLATE("[parallel][bcast] Vectors resize with known and unknown counts", T, unsigned, double) {
  const unsigned rank = mpi_rank(), root = mpi_size() - 1;
  INFO("rank = " << rank);
  const std::vector<T> expected{T{3}, T{1}, T{4}};
  for (const unsigned count : {0u, 3u}) {
    auto buffer = rank == root ? expected : std::vector<T>{T{99}};
    vector_bcast(buffer, root, rank, count);
    CHECK(buffer == expected);
    if (rank == root)
      buffer.clear();
    vector_bcast(buffer, root, rank);
    CHECK(buffer.empty());
  }
}

TEST_CASE("[parallel][bcast] Strings include empty and embedded null content") {
  const unsigned rank = mpi_rank(), root = mpi_size() - 1;
  INFO("rank = " << rank);
  for (const auto &expected : {std::string("TAMRA"), std::string("a\0b", 3), std::string()}) {
    std::string value = rank == root ? expected : "stale output";
    string_bcast(value, root);
    CHECK(value == expected);
  }
}

TEST_CASE("[parallel][bcast] Rectangular matrices with partially known dimensions") {
  const unsigned rank = mpi_rank(), root = mpi_size() - 1;
  INFO("rank = " << rank);
  const std::vector<std::vector<unsigned>> expected{{1, 2, 3}, {4, 5, 6}};
  for (const unsigned rows : {0u, 2u})
    for (const unsigned columns : {0u, 3u}) {
      CAPTURE(rows);
      CAPTURE(columns);
      auto buffer = rank == root ? expected : std::vector<std::vector<unsigned>>{{99}};
      matrix_bcast(buffer, root, rank, rows, columns);
      CHECK(buffer == expected);
    }
}

TEST_CASE("[parallel][bcast] Empty matrices and zero-column rows") {
  const unsigned rank = mpi_rank(), root = mpi_size() - 1;
  INFO("rank = " << rank);
  for (const unsigned rows : {0u, 2u}) {
    const std::vector<std::vector<unsigned>> expected(rows);
    auto buffer = rank == root ? expected : std::vector<std::vector<unsigned>>{{99}};
    matrix_bcast(buffer, root, rank);
    CHECK(buffer == expected);
  }
}
