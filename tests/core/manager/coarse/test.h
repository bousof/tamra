#ifdef USE_MPI

#include "parallel_tests.h"

#else

void registerCoreManagerCoarseParallelTests() {}

#endif // USE_MPI

#include "serial_tests.h"

void registerCoreManagerCoarseTests() {
  registerCoreManagerCoarseSerialTests();
  registerCoreManagerCoarseParallelTests();
}
