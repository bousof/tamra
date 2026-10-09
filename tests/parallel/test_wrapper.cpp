#include <doctest.h>

#include <tamra/parallel/wrapper.h>

#ifdef USE_MPI
#include <tamra/parallel/mpitypes.h>
#endif

TEST_CASE("[parallel][wrapper] Rank size and barrier match the execution mode") {
#ifdef USE_MPI
  int initialized = 0, rank = -1, size = 0;
  MPI_Initialized(&initialized);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  CHECK(initialized == 1);
  CHECK(mpi_rank() == static_cast<unsigned>(rank));
  CHECK(mpi_size() == static_cast<unsigned>(size));
#else
  int argc = 0;
  char **argv = nullptr;
  CHECK(mpi_init(&argc, &argv) == 0);
  CHECK(mpi_rank() == 0);
  CHECK(mpi_size() == 1);
#endif
  CHECK(mpi_barrier() == 0);
#ifndef USE_MPI
  CHECK(mpi_finalize() == 0);
#endif
}

#ifdef USE_MPI
TEST_CASE_TEMPLATE("[parallel][mpitypes] Datatype sizes match C++ storage", T,
                   bool, char, double, float, int, long double, unsigned) {
  int byte_count = 0;
  CHECK(MPI_Type_size(mpi_type<T>(), &byte_count) == MPI_SUCCESS);
  CHECK(static_cast<std::size_t>(byte_count) == sizeof(T));
}
#endif
