//#define USE_MPI

#ifndef USE_MPI

void registerCoreTreeParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreTreeTests() {
  registerCoreTreeSerialTests();
  registerCoreTreeParallelTests();
}
