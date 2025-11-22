#include <parallel/wrapper.h>
#include <UnitTestRegistry.h>

#include "tests.h"

int main(int argc, char** argv) {
  mpi_init(&argc, &argv);

  UnitTestRegistry::label = "P_" + std::to_string(mpi_rank()) + ": ";

  // Registering tests
  registerAllTests();

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
