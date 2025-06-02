//#define USE_MPI

#ifndef USE_MPI

void registerCoreTreeIteratorParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreTreeIteratorTests() {
  registerCoreTreeIteratorSerialTests();
  registerCoreTreeIteratorParallelTests();
}
