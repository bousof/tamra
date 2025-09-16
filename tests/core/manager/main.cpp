#include <parallel/wrapper.h>
#include <testing/UnitTestRegistry.h>
#include "test.h"

int main(int argc, char** argv) {
  mpi_init(&argc, &argv);

  // Registering tests
  registerCoreManagerTests();

  // Running serial tests
  if (mpi_rank() == 0)
    UnitTestRegistry::runSerial();

  mpi_barrier();

  // Running parallel tests
  if (mpi_size() > 1) {
    if (mpi_rank() == 0)
      std::cout << std::endl << std::endl;
    UnitTestRegistry::runParallel(mpi_rank() == 0);
  }

  mpi_finalize();
}
