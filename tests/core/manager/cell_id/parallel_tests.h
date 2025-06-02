#include <mpi.h>

#include <memory>
#include <vector>
#include <core/Cell.h>
#include <core/Tree.h>
#include <core/RootCellEntry.h>
#include <testing/UnitTestRegistry.h>

void registerCoreManagerCellIdParallelTests() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  UnitTestRegistry::label = "P_" + std::to_string(rank) + ": ";
}
