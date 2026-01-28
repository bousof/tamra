#include <doctest.h>

#include <iostream>
#include <vector>

#include <parallel/allreduce.h>
#include <parallel/bcast.h>
#include <parallel/wrapper.h>

// Broadcast vector of unsigned (count known)
TEST_CASE("[communications][bcast] Broadcast vector of unsigned (count known)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  unsigned root = 0, count = 3;

  std::vector<unsigned> buffer;
  if (rank == root)
    buffer = {2, 7, 8};
  vectorBcast<unsigned>(buffer, root, rank, count);

  bool passed = buffer == std::vector<unsigned>({2, 7, 8});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Broadcast vector of unsigned (count unknown)
TEST_CASE("[communications][bcast] Broadcast vector of unsigned (count unknown)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  unsigned root = 0;

  std::vector<unsigned> buffer;
  if (rank == root)
    buffer = {2, 7, 8};
  vectorBcast<unsigned>(buffer, root, rank);

  bool passed = buffer == std::vector<unsigned>({2, 7, 8});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Broadcast matrix of unsigned (count known)
TEST_CASE("[communications][bcast] Broadcast matrix of unsigned (count known)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  unsigned root = 0, rowCount = 4, colCount = 3;

  std::vector<std::vector<unsigned>> buffer;
  if (rank == root)
    buffer = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12} };
  matrixBcast<unsigned>(buffer, root, rank, rowCount, colCount);

  bool passed = buffer[0] == std::vector<unsigned>({1, 2, 3});
  passed &= buffer[1] == std::vector<unsigned>({4, 5, 6});
  passed &= buffer[2] == std::vector<unsigned>({7, 8, 9});
  passed &= buffer[3] == std::vector<unsigned>({10, 11, 12});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Broadcast matrix of unsigned (count unknown)
TEST_CASE("[communications][bcast] Broadcast matrix of unsigned (count unknown)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  unsigned root = 0;

  std::vector<std::vector<unsigned>> buffer;
  if (rank == root)
    buffer = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12} };
  matrixBcast<unsigned>(buffer, root, rank);

  bool passed = buffer[0] == std::vector<unsigned>({1, 2, 3});
  passed &= buffer[1] == std::vector<unsigned>({4, 5, 6});
  passed &= buffer[2] == std::vector<unsigned>({7, 8, 9});
  passed &= buffer[3] == std::vector<unsigned>({10, 11, 12});

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}
