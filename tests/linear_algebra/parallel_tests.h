#ifndef USE_MPI
#  define USE_MPI
#endif
#include <mpi.h>

#include <iostream>
#include <linear_algebra/jacobi.h>
#include <parallel/allreduce.h>
#include <testing/UnitTestRegistry.h>
#include <vector>

bool testJacobiNoOverlapParallel100(int rank, int size);
bool testJacobiOverlapParallel100(int rank, int size);

void registerLinalgParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
  UnitTestRegistry::registerParallelTest("Jacobi parallel (no overlap, 100 iter)", [=]() { return testJacobiNoOverlapParallel100(rank, size); }, "linalg");
  UnitTestRegistry::registerParallelTest("Jacobi parallel (overlap, 100 iter)", [=]() { return testJacobiOverlapParallel100(rank, size); }, "linalg");
}

// Jacobi parallel (no overlap, 100 iter)
// 
//     [ 2 1           ]       [ 2 ]        [ 0 ]         [ -1 ] 
//     [ 1 1      (0)  ]       [ 1 ]        [ | ]         [  3 ] 
//     [     .         ]       [ : ]        [ | ]         [  : ] 
// A = [       .       ] , b = [ : ] , x0 = [ | ] , sol = [  : ] 
//     [         .     ]       [ : ]        [ | ]         [  : ] 
//     [  (0)      2 1 ]       [ 2 ]        [ | ]         [ -1 ] 
//     [           1 1 ]       [ 1 ]        [ 0 ]         [  3 ] 
// 
// dim(A) = (2*size) x (2*size)
bool testJacobiNoOverlapParallel100(int rank, int size) {
  Eigen::SparseMatrix<double, Eigen::RowMajor> A_local(2, 2*size);
  A_local.insert(0, 2*rank) = 2.; A_local.insert(0, 2*rank+1) = 1.;
  A_local.insert(1, 2*rank) = 1.; A_local.insert(1, 2*rank+1) = 1.;
  A_local.makeCompressed();

  std::vector<double> b_local = {1., 2.};
  std::vector<double> x_local = {0., 0.};

  std::vector<double> x_sol = parallelSparseJacobi(A_local, b_local, x_local, 100, rank, size);

  bool passed = true;
  std::vector<double> expected = {-1., 3.};
  for (int i = 0; i < 2; ++i) {
    passed &= std::fabs(x_sol[i] - expected[i]) < 1e-10;
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}

// Jacobi parallel (no overlap, 100 iter)
// 
//     [ 2       1   (0) ]       [ 2 ]        [ 0 ]         [ -1 ] 
//     [   \       \     ]       [ | ]        [ | ]         [  | ] 
//     [     \       \   ]       [ | ]        [ | ]         [  | ] 
// A = [       2  (0)  1 ] , b = [ 2 ] , x0 = [ | ] , sol = [ -1 ] 
//     [ 1  (0)  1       ]       [ 1 ]        [ | ]         [  3 ] 
//     [   \       \     ]       [ | ]        [ | ]         [  | ] 
//     [     \       \   ]       [ | ]        [ | ]         [  | ] 
//     [ (0)   1       1 ]       [ 1 ]        [ 0 ]         [  3 ] 
// 
// dim(A) = (2*size) x (2*size)
bool testJacobiOverlapParallel100(int rank, int size) {
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
  for (int i = 0; i < 2; ++i) {
    passed &= std::fabs(x_sol[i] - expected[i]) < 1e-10;
  }

  // Test should pass on all processes
  bool all_passed;
  boolAndAllReduce(passed, all_passed);
  return all_passed;
}
