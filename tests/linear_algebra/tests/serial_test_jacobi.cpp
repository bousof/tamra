#include <doctest.h>
#include <test_macros.h>

#include <iostream>
#include <vector>
#include <linear_algebra/jacobi.h>

// Jacobi 2x2 serial (1 iter)
//
//     [ 2  1 ]       [ 2 ]        [ 0 ]        [ 1/2 ]
// A = [      ] , b = [   ] , x0 = [   ] , x1 = [     ]
//     [ 1  1 ]       [ 1 ]        [ 0 ]        [  2  ]
//
TEST_CASE("[linalg][jacobi] Jacobi 2x2 serial (1 iter)") {
  Eigen::SparseMatrix<double, Eigen::RowMajor> A(2, 2);
  A.insert(0, 0) = 2.; A.insert(0, 1) = 1.;
  A.insert(1, 0) = 1.; A.insert(1, 1) = 1.;
  A.makeCompressed();

  std::vector<double> b = {1., 2.};
  std::vector<double> x = {0., 0.};

  std::vector<double> x_next = jacobiIteration(A, b, x);

  std::vector<double> expected = {0.5, 2.0};
  for (int i{0}; i<2; ++i)
    CHECK(std::fabs(x_next[i] - expected[i]) < 1e-10);
}

// Jacobi 2x2 serial (100 iter)
//
//     [ 2  1 ]       [ 2 ]        [ 0 ]         [ -1 ]
// A = [      ] , b = [   ] , x0 = [   ] , sol = [    ]
//     [ 1  1 ]       [ 1 ]        [ 0 ]         [  3 ]
//
TEST_CASE("[linalg][jacobi] Jacobi 2x2 serial (100 iter)") {
  // 2x2 diagonally dominant matrix
  Eigen::SparseMatrix<double, Eigen::RowMajor> A(2, 2);
  A.insert(0, 0) = 2.; A.insert(0, 1) = 1.;
  A.insert(1, 0) = 1.; A.insert(1, 1) = 1.;
  A.makeCompressed();

  std::vector<double> b = {1., 2.};
  std::vector<double> x = {0., 0.};

  std::vector<double> x_sol = sparseJacobi(A, b, x, 100);

  bool passed = true;
  std::vector<double> expected = {-1., 3.};
  for (int i{0}; i<2; ++i)
    CHECK(std::fabs(x_sol[i] - expected[i]) < 1e-10);
}
