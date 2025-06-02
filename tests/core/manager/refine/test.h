//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerRefineParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreManagerRefineTests() {
  registerCoreManagerRefineSerialTests();
  registerCoreManagerRefineParallelTests();
}
