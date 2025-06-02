//#define USE_MPI

#ifdef USE_MPI

#include <mpi.h>
int mpi_init() {
  // MPI initialization
  return MPI_Init(NULL, NULL);
}
int mpi_rank() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return rank;
}
int mpi_size() {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return size;
}
int mpi_barrier() {
  // MPI initialization
  return MPI_Barrier(MPI_COMM_WORLD);
}
int mpi_finalize() {
  // MPI finalization
  return MPI_Finalize();
}

#else

// MPI initialization
int mpi_init() {return 0;}
int mpi_rank() {return 0;}
int mpi_size() {return 1;}
int mpi_barrier() {return 0;}
int mpi_finalize() {return 0;}

#endif

#include <testing/UnitTestRegistry.h>
#include "tests.h"

int main(int argc, char** argv) {
  mpi_init();

  // Registering tests
  registerCommunicationsTests();

  // Running serial tests
  if (mpi_rank()==0) {
    UnitTestRegistry::runSerial();
    std::cout << std::endl << std::endl;
  }

  mpi_barrier();

  // Running parallel tests
  if (mpi_size()>1)
    UnitTestRegistry::runParallel(mpi_rank()==0);

  mpi_finalize();
}
