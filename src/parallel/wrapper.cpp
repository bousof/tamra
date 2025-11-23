#include "../../includes/parallel/wrapper.h"

// MPI initialization
int mpi_init(int *argc, char ***argv) {
#ifdef USE_MPI
  return MPI_Init(argc, argv);
#else
  return 0;
#endif // USE_MPI
}

// MPI rank
unsigned mpi_rank() {
#ifdef USE_MPI
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  return (unsigned)rank;
#else
  return 0;
#endif // USE_MPI
}

// MPI size
unsigned mpi_size() {
#ifdef USE_MPI
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  return (unsigned)size;
#else
  return 1;
#endif // USE_MPI
}

// MPI barrier
int mpi_barrier() {
#ifdef USE_MPI
  return MPI_Barrier(MPI_COMM_WORLD);
#else
  return 0;
#endif // USE_MPI
}

// MPI finalization
int mpi_finalize() {
#ifdef USE_MPI
  return MPI_Finalize();
#else
  return 0;
#endif // USE_MPI
}
