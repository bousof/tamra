#pragma once

#ifdef USE_MPI

#include <mpi.h>

// MPI initialization
int mpi_init(int *argc, char ***argv) {
  return MPI_Init(argc, argv);
}

// MPI rank
unsigned mpi_rank() {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return (unsigned)rank;
}

// MPI size
unsigned mpi_size() {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return (unsigned)size;
}

// MPI barrier
int mpi_barrier() {
  return MPI_Barrier(MPI_COMM_WORLD);
}

// MPI finalization
int mpi_finalize() {
  return MPI_Finalize();
}

#else

// MPI initialization
int mpi_init(int *, char ***) {
  return 0;
}

// MPI rank
unsigned mpi_rank() {
  return 0;
}

// MPI size
unsigned mpi_size() {
  return 1;
}

// MPI barrier
int mpi_barrier() {
  return 0;
}

// MPI finalization
int mpi_finalize() {
  return 0;
}

#endif // USE_MPI
