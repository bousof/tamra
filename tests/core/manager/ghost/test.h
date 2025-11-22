#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerGhostParallelTests() {}

#endif // USE_MPI

void registerCoreManagerGhostTests() {
  registerCoreManagerGhostParallelTests();
}
