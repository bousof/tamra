//#define USE_MPI

#ifndef USE_MPI

void registerCoreManagerCellIdParallelTests() {}

#else

#include "parallel_tests.h"

#endif

#include "serial_tests.h"

void registerCoreManagerCellIdTests() {
  registerCoreManagerCellIdSerialTests();
  registerCoreManagerCellIdParallelTests();
}
