//#define USE_MPI

#ifndef USE_MPI

void registerCoreCellOctParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreCellOctTests() {
  registerCoreCellOctSerialTests();
  registerCoreCellOctParallelTests();
}
