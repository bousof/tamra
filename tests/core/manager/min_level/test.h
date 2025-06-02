//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerMinLevelParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreManagerMinLevelTests() {
  registerCoreManagerMinLevelSerialTests();
  registerCoreManagerMinLevelParallelTests();
}
