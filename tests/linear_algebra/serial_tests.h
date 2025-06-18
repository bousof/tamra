#include <iostream>
#include <vector>
#include <linear_algebra/jacobi.h>
#include <testing/UnitTestRegistry.h>

bool testJacobi2x2Serial1();
bool testJacobi2x2Serial100();

void registerLinalgSerialTests() {
  UnitTestRegistry::registerSerialTest("Jacobi 2x2 serial (1 iter)", [=]() { return testJacobi2x2Serial1(); }, "linalg");
  UnitTestRegistry::registerSerialTest("Jacobi 2x2 serial (100 iter)", [=]() { return testJacobi2x2Serial100(); }, "linalg");
}

// Jacobi 2x2 serial (1 iter)
//
//     [ 2  1 ]       [ 2 ]        [ 0 ]        [ 1/2 ]
// A = [      ] , b = [   ] , x0 = [   ] , x1 = [     ]
//     [ 1  1 ]       [ 1 ]        [ 0 ]        [  2  ]
//
bool testJacobi2x2Serial1() {
  Eigen::SparseMatrix<double, Eigen::RowMajor> A(2, 2);
  A.insert(0, 0) = 2.; A.insert(0, 1) = 1.;
  A.insert(1, 0) = 1.; A.insert(1, 1) = 1.;
  A.makeCompressed();

  std::vector<double> b = {1., 2.};
  std::vector<double> x = {0., 0.};

  std::vector<double> x_next = jacobiIteration(A, b, x);

  bool passed = true;
  std::vector<double> expected = {0.5, 2.0};
  for (int i = 0; i < 2; ++i) {
    passed &= std::fabs(x_next[i] - expected[i]) < 1e-10;
  }
  return passed;
}

// Jacobi 2x2 serial (100 iter)
//
//     [ 2  1 ]       [ 2 ]        [ 0 ]         [ -1 ]
// A = [      ] , b = [   ] , x0 = [   ] , sol = [    ]
//     [ 1  1 ]       [ 1 ]        [ 0 ]         [  3 ]
//
bool testJacobi2x2Serial100() {
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
  for (int i = 0; i < 2; ++i) {
    passed &= std::fabs(x_sol[i] - expected[i]) < 1e-10;
  }
  return passed;
}
