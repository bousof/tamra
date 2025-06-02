#include <mpi.h>

#include <iostream>
#include <vector>
#include <linear_algebra/jacobi.h>
#include <testing/UnitTestRegistry.h>

void registerCoreCellOctParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
}
