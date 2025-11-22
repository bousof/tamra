#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreTreeParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreTreeTests() {
  registerCoreTreeSerialTests();
  registerCoreTreeParallelTests();
}
