//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerCoarseParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreManagerCoarseTests() {
  registerCoreManagerCoarseSerialTests();
  registerCoreManagerCoarseParallelTests();
}
