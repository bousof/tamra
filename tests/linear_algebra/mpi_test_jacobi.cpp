#include <doctest.h>

#include <iostream>
#include <vector>

#include <linear_algebra/jacobi.h>
#include <parallel/allreduce.h>
#include <parallel/wrapper.h>

// Jacobi parallel (no overlap, 100 iter)
//
//     ┌ 2 1           ┐       ┌ 2 ┐        ┌ 0 ┐         ┌ -1 ┐
//     │ 1 1      (0)  │       │ 1 │        │ | │         │  3 │
//     │     .         │       │ : │        │ | │         │  : │
// A = │       .       │ , b = │ : │ , x0 = │ | │ , sol = │  : │
//     │         .     │       │ : │        │ | │         │  : │
//     │  (0)      2 1 │       │ 2 │        │ | │         │ -1 │
//     └           1 1 ┘       └ 1 ┘        └ 0 ┘         └  3 ┘
//
// dim(A) = (2*size) x (2*size)
TEST_CASE("[linalg][jacobi][mpi] Jacobi parallel (no overlap, 100 iter)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  Eigen::SparseMatrix<double, Eigen::RowMajor> A_local(2, 2*size);
  A_local.insert(0, 2*rank) = 2.; A_local.insert(0, 2*rank+1) = 1.;
  A_local.insert(1, 2*rank) = 1.; A_local.insert(1, 2*rank+1) = 1.;
  A_local.makeCompressed();

  std::vector<double> b_local = {1., 2.};
  std::vector<double> x_local = {0., 0.};

  std::vector<double> x_sol = parallelSparseJacobi(A_local, b_local, x_local, 100, rank, size);

  bool passed = true;
  std::vector<double> expected = {-1., 3.};
  for (int i{0}; i<2; ++i)
    passed &= std::fabs(x_sol[i] - expected[i]) < 1e-10;

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}

// Jacobi parallel (no overlap, 100 iter)
//
//     ┌ 2       1   (0) ┐       ┌ 2 ┐        ┌ 0 ┐         ┌ -1 ┐
//     │   \       \     │       │ | │        │ | │         │  | │
//     │     \       \   │       │ | │        │ | │         │  | │
// A = │       2  (0)  1 │ , b = │ 2 │ , x0 = │ | │ , sol = │ -1 │
//     │ 1  (0)  1       │       │ 1 │        │ | │         │  3 │
//     │   \       \     │       │ | │        │ | │         │  | │
//     │     \       \   │       │ | │        │ | │         │  | │
//     └ (0)   1       1 ┘       └ 1 ┘        └ 0 ┘         └  3 ┘
//
// dim(A) = (2*size) x (2*size)
TEST_CASE("[linalg][jacobi][mpi] Jacobi parallel (overlap, 100 iter)") {
  const unsigned rank = mpi_rank(),
                 size = mpi_size();

  // 3x3 diagonally dominant matrix
  Eigen::SparseMatrix<double, Eigen::RowMajor> A_local(2, 2*size);
  A_local.insert(0, (2*rank)%size)        = 2. - (2*rank)/size;
  A_local.insert(0, (2*rank)%size+size)   = 1.;
  A_local.insert(1, (2*rank+1)%size)      = 2. - (2*rank+1)/size;
  A_local.insert(1, (2*rank+1)%size+size) = 1.;
  A_local.makeCompressed();

  std::vector<double> b_local = {
    1. + (2*rank)/size,
    1. + (2*rank+1)/size
  };
  std::vector<double> x_local = {0., 0.};

  std::vector<double> x_sol = parallelSparseJacobi(A_local, b_local, x_local, 100, rank, size);

  bool passed = true;
  std::vector<double> expected = {
    -1. + 4*((2*rank)/size),
    -1. + 4*((2*rank+1)/size)
  };
  for (int i{0}; i<2; ++i)
    passed &= std::fabs(x_sol[i] - expected[i]) < 1e-10;

  // Test should pass on all processes
  bool all_passed;
  boolAndAllreduce(passed, all_passed);

  // Final check
  CHECK(all_passed);
}
