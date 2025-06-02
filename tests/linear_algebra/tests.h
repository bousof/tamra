//#define USE_MPI

#ifndef USE_MPI

void registerLinalgParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerLinalgTests() {
  registerLinalgSerialTests();
  registerLinalgParallelTests();
}
